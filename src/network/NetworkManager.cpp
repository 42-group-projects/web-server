#include "NetworkManager.hpp"
#include "../http/HttpHandler/HttpHandler.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"

NetworkManager::NetworkManager(const ServerConfig& config) : config(config), running(false) {}

NetworkManager::~NetworkManager() {
    for (size_t i = 0; i < pollfds.size(); ++i) {
        close(pollfds[i].fd);
    }
}

bool NetworkManager::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return false;
    return true;
}

void NetworkManager::addPollFd(int fd, short events, bool listener)
{
    struct pollfd p; p.fd = fd; p.events = events; p.revents = 0;
    pollfds.push_back(p);
    isListener[fd] = listener;
}

void NetworkManager::removeFd(int fd)
{
    close(fd);
    for (std::vector<struct pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it) {
        if (it->fd == fd) { pollfds.erase(it); break; }
    }
    isListener.erase(fd);
    states.erase(fd);
    sendBufs.erase(fd);
}

bool NetworkManager::addListener(const std::string &ip, int port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return false;

    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (!setNonBlocking(fd)) { close(fd); return false; }

    struct sockaddr_in addr; std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        close(fd); return false;
    }

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd); return false;
    }

    if (listen(fd, 128) < 0) {
        close(fd); return false;
    }

    listeners.push_back(fd);
    addPollFd(fd, POLLIN, true);
    std::cout << "Listening on " << ip << ":" << port << " (fd=" << fd << ")" << std::endl;
    return true;
}

bool NetworkManager::init()
{
    const std::vector<std::pair<std::string, int> > &lst = config.getAllListen();
    if (lst.empty()) {
        std::cerr << "No listen directives found in config." << std::endl;
        return false;
    }

    bool anyFailure = false;
    for (size_t i = 0; i < lst.size(); ++i) {
        if (!addListener(lst[i].first, lst[i].second)) {
            std::cerr << "Failed to add listener for " << lst[i].first << ":" << lst[i].second << std::endl;
            anyFailure = true;
        }
    }
    if (listeners.empty()) {
        running = false;
        return false;
    }
    if (anyFailure) {
        std::cerr << "One or more listeners failed to start, continuing with available listeners." << std::endl;
    }
    running = true;
    return true;
}

void NetworkManager::stop()
{
    running = false;
}

void NetworkManager::touchActivity(int fd)
{
    std::map<int, ConnState>::iterator it = states.find(fd);
    if (it != states.end())
        it->second.lastActivity = std::time(NULL);
}

void NetworkManager::cleanupIdleConnections()
{
    time_t now = std::time(NULL);

    // NOTE: removeFd() erases from pollfds; vector iterators would become invalid.
    // Iterate by index to safely erase while scanning.
    for (size_t i = 0; i < pollfds.size(); )
    {
        int fd = pollfds[i].fd;
        if (isListener.count(fd) && isListener[fd]) {
            ++i;
            continue;
        }

        std::map<int, ConnState>::iterator stIt = states.find(fd);
        if (stIt == states.end()) {
            ++i;
            continue;
        }

        ConnState &st = stIt->second;
        if (st.lastActivity != 0 && (now - st.lastActivity) > kKeepAliveTimeoutSec)
        {
            removeFd(fd);
            // pollfds shrank; do not increment i
            continue;
        }
        ++i;
    }
}

bool NetworkManager::shouldKeepAlive(const std::string &requestHead) const
{
    std::map<std::string, std::string> headers = parseHeaderMap(requestHead);
    std::map<std::string, std::string>::iterator it = headers.find("connection");
    if (it == headers.end())
        return false;

    std::string v = trim(toLower(it->second));
    if (v.find("close") != std::string::npos)
        return false;
    if (v.find("keep-alive") != std::string::npos)
        return true;
    return false;
}

bool NetworkManager::isBodyLengthRequiredError(const std::string &requestHead) const
{
    size_t lineEnd = requestHead.find("\r\n");
    if (lineEnd == std::string::npos)
        return false;

    std::string requestLine = requestHead.substr(0, lineEnd);
    std::string method;
    {
        std::istringstream iss(requestLine);
        iss >> method;
    }

    if (!(method == "POST" || method == "PUT" || method == "PATCH"))
        return false;

    std::map<std::string, std::string> headers = parseHeaderMap(requestHead.substr(lineEnd + 2));

    bool isChunked = false;
    bool hasCL = false;

    std::map<std::string, std::string>::iterator it = headers.find("transfer-encoding");
    if (it != headers.end()) {
        std::string v = trim(toLower(it->second));
        if (v.find("chunked") != std::string::npos)
            isChunked = true;
    }

    it = headers.find("content-length");
    if (it != headers.end()) {
        std::string v = trim(it->second);
        if (!v.empty())
            hasCL = true;
    }

    return (!isChunked && !hasCL);
}

void NetworkManager::handleListenerEvent(int fd)
{
    while (true) {
        struct sockaddr_in cli; socklen_t len = sizeof(cli);
        int cfd = accept(fd, (struct sockaddr*)&cli, &len);
        if (cfd < 0) {
            break;
        }
        if (!setNonBlocking(cfd)) { close(cfd); continue; }
        addPollFd(cfd, POLLIN, false);

        ConnState &st = states[cfd];
        st.buf.clear();
        st.headerDone = false;
        st.headerEndPos = 0;
        st.isChunked = false;
        st.contentLength = (size_t)-1;
        st.chunkParsePos = 0;
        st.wantClose = true;
        st.requestCount = 0;
        st.lastActivity = std::time(NULL);

        char ipbuf[INET_ADDRSTRLEN];
        const char *p = inet_ntop(AF_INET, &cli.sin_addr, ipbuf, sizeof(ipbuf));

        clientIps[cfd] = std::string(p);
        struct sockaddr_in localAddr;
        socklen_t addrlen = sizeof(localAddr);
        if (getsockname(cfd, (struct sockaddr*)&localAddr, &addrlen) == 0)
        {
            int serverPort = ntohs(localAddr.sin_port);
            clientPorts[cfd] = serverPort;
        }
        std::cout 
            << "Accepted fd=" << cfd
            << " from " << (p ? p : "?") << ":" << ntohs(cli.sin_port)
            << " on port " << clientPorts[cfd] << std::endl;
    }
}

void NetworkManager::queueSimpleOkResponse(int fd)
{
    static const char body[] = "Hello from webserv\n";
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Server: webserv\r\n";
    oss << "Host: " << getServerName(clientPorts[fd]) << "\r\n";
    oss << "Content-Type: text/plain\r\n";
    oss << "Content-Length: " << sizeof(body) - 1 << "\r\n";
    oss << "Connection: close\r\n\r\n";
    std::string header = oss.str();
    sendBufs[fd] = header + std::string(body, sizeof(body) - 1);

    for (size_t i = 0; i < pollfds.size(); ++i) {
        if (pollfds[i].fd == fd) {
            pollfds[i].events |= POLLOUT;
            break;
        }
    }
}

void NetworkManager::handleClientRead(int fd)
{
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n == 0) {
        // Peer performed an orderly shutdown (FIN). If we already have buffered data,
        // keep the fd around long enough to parse and respond, then close.
        std::map<int, ConnState>::iterator it = states.find(fd);
        if (it != states.end() && !it->second.buf.empty()) {
            it->second.wantClose = true;
            while (tryParseRequest(fd)) {
            }
            return;
        }
        removeFd(fd);
        return;
    }
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        removeFd(fd);
        return;
    }

    touchActivity(fd);

    ConnState &st = states[fd];
    st.buf.append(buf, n);

    // Protect against unbounded memory usage (header/body abuse)
    if (st.buf.size() > kMaxHeaderBytes + kMaxBodyBytes) {
        HttpResponse res;
        res.setVersion("HTTP/1.1");
        res.setHeader("Server", "webserv");
        res.setHeader("Host", getServerName(clientPorts[fd]));
        res.setStatus(CONTENT_TOO_LARGE);
        res.setMimeType("text/plain");
        res.setBody("Payload Too Large\n");
        st.wantClose = true;
        res.setHeader("Connection", "close");

        sendBufs[fd] = res.generateResponse(res.getStatus());
        for (size_t i = 0; i < pollfds.size(); ++i)
            if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }
        return;
    }

    while (tryParseRequest(fd)) {
    }
}

void NetworkManager::handleClientWrite(int fd)
{
    std::map<int, std::string>::iterator it = sendBufs.find(fd);
    if (it == sendBufs.end()) {
        for (size_t i = 0; i < pollfds.size(); ++i) {
            if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLOUT; break; }
        }
        return;
    }

    const std::string &data = it->second;
    ssize_t n = send(fd, data.data(), data.size(), 0);
    if (n == 0) {
        return;
    }
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        removeFd(fd);
        return;
    }

    touchActivity(fd);

    if ((size_t)n >= data.size()) {
        sendBufs.erase(fd);

        ConnState &st = states[fd];
        bool closeAfter = st.wantClose;
        st.wantClose = true;

        if (closeAfter) {
            removeFd(fd);
            return;
        }

        for (size_t i = 0; i < pollfds.size(); ++i) {
            if (pollfds[i].fd == fd) {
                pollfds[i].events &= ~POLLOUT;
                pollfds[i].events |= POLLIN;
                break;
            }
        }

        // Drain any data already waiting in the kernel receive buffer.
        // This avoids depending on a new POLLIN notification when the client already sent
        // the next request before we re-enabled reading.
        while (true) {
            char rbuf[4096];
            ssize_t rn = recv(fd, rbuf, sizeof(rbuf), 0);
            if (rn > 0) {
                touchActivity(fd);
                states[fd].buf.append(rbuf, rn);
                if (states[fd].buf.size() > kMaxHeaderBytes + kMaxBodyBytes) {
                    HttpResponse res;
                    res.setVersion("HTTP/1.1");
                    res.setHeader("Server", "webserv");
                    res.setHeader("Host", getServerName(clientPorts[fd]));
                    res.setStatus(CONTENT_TOO_LARGE);
                    res.setMimeType("text/plain");
                    res.setBody("Payload Too Large\n");
                    states[fd].wantClose = true;
                    res.setHeader("Connection", "close");

                    sendBufs[fd] = res.generateResponse(res.getStatus());
                    for (size_t j = 0; j < pollfds.size(); ++j)
                        if (pollfds[j].fd == fd) { pollfds[j].events &= ~POLLIN; pollfds[j].events |= POLLOUT; break; }
                    break;
                }
                continue;
            }
            if (rn == 0) {
                // peer closed
                removeFd(fd);
                return;
            }
            if (rn < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                removeFd(fd);
                return;
            }
        }

        // Now parse any buffered requests.
        while (tryParseRequest(fd)) {
        }

        return;
    }

    it->second.erase(0, n);
}

void NetworkManager::run()
{
    if (!running) return;
    std::cout << "Network loop started." << std::endl;
    while (running) {
        cleanupIdleConnections();

        int ret = poll(&pollfds[0], pollfds.size(), 1000);
        if (ret < 0) {
            continue;
        }
        if (ret == 0) {
            continue;
        }

        std::vector<struct pollfd> current = pollfds;
        for (size_t i = 0; i < current.size(); ++i) {
            int fd = current[i].fd;
            short re = current[i].revents;
            if (re == 0) continue;

            if (re & (POLLHUP | POLLERR | POLLNVAL)) {
                removeFd(fd);
                continue;
            }

            if (isListener.count(fd) && isListener[fd]) {
                if (re & POLLIN) handleListenerEvent(fd);
                continue;
            }

            if (re & POLLIN) handleClientRead(fd);
            if (re & POLLOUT) handleClientWrite(fd);
        }
    }
}

bool NetworkManager::tryParseRequest(int fd)
{
    ConnState &st = states[fd];

    if (sendBufs.find(fd) != sendBufs.end())
        return false;

    if (!st.headerDone) {
        // Header size guard: if we haven't found CRLFCRLF yet, cap buffer growth
        if (st.buf.size() > kMaxHeaderBytes) {
            HttpResponse res;
            res.setVersion("HTTP/1.1");
            res.setHeader("Server", "webserv");
            res.setHeader("Host", getServerName(clientPorts[fd]));
            res.setStatus(REQUEST_HEADER_FIELDS_TOO_LARGE);
            res.setMimeType("text/plain");
            res.setBody("Request Header Fields Too Large\n");
            st.wantClose = true;
            res.setHeader("Connection", "close");

            sendBufs[fd] = res.generateResponse(res.getStatus());
            for (size_t i = 0; i < pollfds.size(); ++i)
                if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }
            // Drop buffered junk; we will close after sending
            st.buf.clear();
            st.headerDone = false;
            st.headerEndPos = 0;
            st.isChunked = false;
            st.contentLength = (size_t)-1;
            st.chunkParsePos = 0;
            return false;
        }

        size_t pos = st.buf.find("\r\n\r\n");
        if (pos == std::string::npos) return false;
        st.headerDone = true;
        st.headerEndPos = pos + 4;
        parseHeadersAndInitState(st);

        std::cerr << "[NM][fd=" << fd << "] header parsed: headerEndPos=" << st.headerEndPos
                  << " buf.size=" << st.buf.size() << " isChunked=" << (st.isChunked ? 1 : 0)
                  << " contentLength=" << (st.contentLength == (size_t)-1 ? -1 : (long long)st.contentLength)
                  << std::endl;
    }

    bool complete = false;
    size_t totalConsumed = 0;

    std::string requestHead = st.buf.substr(0, st.headerEndPos);

    {
        size_t lineEnd = requestHead.find("\r\n");
        std::string rl = (lineEnd == std::string::npos) ? requestHead : requestHead.substr(0, lineEnd);
        std::cerr << "[NM][fd=" << fd << "] requestLine='" << rl << "'" << std::endl;
    }

    bool lengthRequired = isBodyLengthRequiredError(requestHead);

    if (lengthRequired) {
        HttpResponse res;
        res.setVersion("HTTP/1.1");
        res.setHeader("Server", "webserv");
        res.setHeader("Host", getServerName(clientPorts[fd]));
        res.setStatus(LENGTH_REQUIRED);
        res.setMimeType("text/plain");
        res.setBody("Length Required\n");

        bool keep = shouldKeepAlive(requestHead);

        st.requestCount++;
        if (st.requestCount >= kMaxRequestsPerConnection)
            keep = false;

        st.wantClose = !keep;
        res.setHeader("Connection", keep ? "keep-alive" : "close");

        sendBufs[fd] = res.generateResponse(res.getStatus());
        for (size_t i = 0; i < pollfds.size(); ++i)
            if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }

        totalConsumed = st.headerEndPos;
        complete = true;
    }
    else if (st.isChunked) {
        std::string decoded;
        ChunkDecodeResult r = tryDecodeChunked(st, decoded, totalConsumed);
        std::cerr << "[NM][fd=" << fd << "] chunked decode result=" << (int)r
                  << " decoded.size=" << decoded.size() << " totalConsumed=" << totalConsumed
                  << " buf.size=" << st.buf.size() << std::endl;
        if (r == CHUNK_NEED_MORE) {
            // If chunked body grows too large without completing, reject to avoid memory exhaustion
            if (st.buf.size() > kMaxHeaderBytes + kMaxBodyBytes) {
                HttpResponse res;
                res.setVersion("HTTP/1.1");
                res.setHeader("Server", "webserv");
                res.setHeader("Host", getServerName(clientPorts[fd]));
                res.setStatus(CONTENT_TOO_LARGE);
                res.setMimeType("text/plain");
                res.setBody("Payload Too Large\n");
                st.wantClose = true;
                res.setHeader("Connection", "close");

                sendBufs[fd] = res.generateResponse(res.getStatus());
                for (size_t i = 0; i < pollfds.size(); ++i)
                    if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }
            }
            return false;
        }
        if (r == CHUNK_INVALID) {
            HttpResponse res;
            res.setVersion("HTTP/1.1");
            res.setHeader("Server", "webserv");
            res.setHeader("Host", getServerName(clientPorts[fd]));
            res.setStatus(BAD_REQUEST);
            res.setMimeType("text/plain");
            res.setBody("Bad Request\n");
            st.wantClose = true;
            res.setHeader("Connection", "close");

            sendBufs[fd] = res.generateResponse(res.getStatus());
            for (size_t i = 0; i < pollfds.size(); ++i)
                if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }

            // Consume the header at least, so we don't spin; we will close after sending
            totalConsumed = st.headerEndPos;
            complete = true;
        } else {
            if (decoded.size() > kMaxBodyBytes) {
                HttpResponse res;
                res.setVersion("HTTP/1.1");
                res.setHeader("Server", "webserv");
                res.setHeader("Host", getServerName(clientPorts[fd]));
                res.setStatus(CONTENT_TOO_LARGE);
                res.setMimeType("text/plain");
                res.setBody("Payload Too Large\n");
                st.wantClose = true;
                res.setHeader("Connection", "close");

                sendBufs[fd] = res.generateResponse(res.getStatus());
                for (size_t i = 0; i < pollfds.size(); ++i)
                    if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }

                totalConsumed = (totalConsumed > 0) ? totalConsumed : st.headerEndPos;
                complete = true;
            } else {
                HttpRequest req;
                try { req.parseRequest(requestHead); } catch(...) {}
                req.setBody(decoded);
                {
                    std::ostringstream cl; cl << decoded.size();
                    req.setHeader("Content-Length", cl.str());
                }

                HttpHandler handler;
                HttpResponse res = handler.handleRequest(req, config, clientIps[fd], clientPorts[fd]);

                if (res.getVersion().empty() || res.getVersion().find("HTTP/") != 0)
                    res.setVersion("HTTP/1.1");
                if (res.getStatus() == UNSET) {
                    res.setStatus(INTERNAL_SERVER_ERROR);
                    if (res.getMimeType().empty()) res.setMimeType("text/plain");
                    if (res.getBody().empty()) res.setBody("Internal Server Error\n");
                }
                res.setHeader("Server", "webserv");
                res.setHeader("Host", getServerName(clientPorts[fd]));

                bool keep = shouldKeepAlive(requestHead);
                st.requestCount++;
                if (st.requestCount >= kMaxRequestsPerConnection)
                    keep = false;

                st.wantClose = !keep;
                res.setHeader("Connection", keep ? "keep-alive" : "close");

                sendBufs[fd] = res.generateResponse(res.getStatus());
                for (size_t i = 0; i < pollfds.size(); ++i)
                    if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }

                complete = true;
            }
        }

        // Clamp totalConsumed to the end of the chunked body/trailer, but never past the current buffer size.
        if (complete) {
            if (totalConsumed > st.buf.size())
                totalConsumed = st.buf.size();
        }
    }
    else {
        size_t bodyHave = (st.buf.size() >= st.headerEndPos) ? (st.buf.size() - st.headerEndPos) : 0;
        size_t need = (st.contentLength == (size_t)-1) ? 0 : st.contentLength;
        std::cerr << "[NM][fd=" << fd << "] non-chunked bodyHave=" << bodyHave << " need=" << need
                  << " headerEndPos=" << st.headerEndPos << " buf.size=" << st.buf.size() << std::endl;
        if (bodyHave >= need) {
            totalConsumed = st.headerEndPos + need;
            std::string body = (need == 0) ? std::string() : st.buf.substr(st.headerEndPos, need);

            HttpRequest req;
            try { req.parseRequest(requestHead); } catch(...) {}
            if (!body.empty()) {
                req.setBody(body);
                std::ostringstream cl; cl << body.size();
                req.setHeader("Content-Length", cl.str());
            }

            HttpHandler handler;
            HttpResponse res = handler.handleRequest(req, config, clientIps[fd], clientPorts[fd]);

            if (res.getVersion().empty() || res.getVersion().find("HTTP/") != 0)
                res.setVersion("HTTP/1.1");
            if (res.getStatus() == UNSET) {
                res.setStatus(INTERNAL_SERVER_ERROR);
                if (res.getMimeType().empty()) res.setMimeType("text/plain");
                if (res.getBody().empty()) res.setBody("Internal Server Error\n");
            }
            res.setHeader("Server", "webserv");
            res.setHeader("Host", getServerName(clientPorts[fd]));

            bool keep = shouldKeepAlive(requestHead);
            st.requestCount++;
            if (st.requestCount >= kMaxRequestsPerConnection)
                keep = false;

            st.wantClose = !keep;
            res.setHeader("Connection", keep ? "keep-alive" : "close");

            sendBufs[fd] = res.generateResponse(res.getStatus());
            for (size_t i = 0; i < pollfds.size(); ++i)
                if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }

            complete = true;
        }
    }

    if (!complete) return false;

    touchActivity(fd);

    std::cerr << "[NM][fd=" << fd << "] complete, erasing consumed bytes=" << totalConsumed
              << " from buf.size=" << st.buf.size() << std::endl;

    if (totalConsumed > 0 && totalConsumed <= st.buf.size())
        st.buf.erase(0, totalConsumed);

    if (!st.buf.empty()) {
        std::string peek = st.buf.substr(0, st.buf.size() > 80 ? 80 : st.buf.size());
        for (size_t i = 0; i < peek.size(); ++i) {
            if (peek[i] == '\r') peek[i] = 'R';
            else if (peek[i] == '\n') peek[i] = 'N';
        }
        std::cerr << "[NM][fd=" << fd << "] remaining buf.size=" << st.buf.size() << " peek(80)='" << peek << "'" << std::endl;
    } else {
        std::cerr << "[NM][fd=" << fd << "] remaining buf empty" << std::endl;
    }

    st.headerDone = false;
    st.headerEndPos = 0;
    st.isChunked = false;
    st.contentLength = (size_t)-1;
    st.chunkParsePos = 0;

    return true;
}

std::map<std::string, std::string> NetworkManager::parseHeaderMap(const std::string &headersRaw)
{
    std::map<std::string, std::string> m;
    size_t start = 0, end;
    while ((end = headersRaw.find("\r\n", start)) != std::string::npos) {
        std::string line = headersRaw.substr(start, end - start);
        start = end + 2;
        if (line.empty()) break;
        size_t colon = line.find(':');
        if (colon == std::string::npos) continue;
        std::string key = line.substr(0, colon);
        std::string val = line.substr(colon + 1);
        // trim
        while (!val.empty() && (val[0] == ' ' || val[0] == '\t')) val.erase(0, 1);
        // lower key
        for (size_t i = 0; i < key.size(); ++i) key[i] = std::tolower(key[i]);
        m[key] = val;
    }
    return m;
}

void NetworkManager::parseHeadersAndInitState(ConnState &st)
{
    if (st.headerEndPos > st.buf.size()) return;
    std::string head = st.buf.substr(0, st.headerEndPos);

    size_t firstCrlf = head.find("\r\n");
    std::string headerOnly = (firstCrlf == std::string::npos) ? head : head.substr(firstCrlf + 2);
    std::map<std::string, std::string> headers = parseHeaderMap(headerOnly);

    st.isChunked = false;
    st.contentLength = (size_t)-1;
    st.chunkParsePos = st.headerEndPos;

    std::map<std::string, std::string>::iterator it;
    it = headers.find("transfer-encoding");
    if (it != headers.end()) {
        std::string v = trim(toLower(it->second));
        if (v.find("chunked") != std::string::npos) {
            st.isChunked = true;
        }
    }
    it = headers.find("content-length");
    if (it != headers.end()) {
        std::string v = trim(it->second);
        std::istringstream iss(v);
        size_t n; if (iss >> n) st.contentLength = n;
    }
}

std::string NetworkManager::getServerName(int port) const
{
    const std::vector<t_server_config>& configs = config.getConfiguration();
    for (size_t i = 0; i < configs.size(); ++i) {
        for (size_t j = 0; j < configs[i].listen.size(); ++j) {
            if (configs[i].listen[j].second == port) {
                if (!configs[i].server_name.empty())
                    return configs[i].server_name[0];
            }
        }
    }
    return "localhost";
}

std::string NetworkManager::toLower(const std::string &s)
{
    std::string r(s);
    for (size_t i = 0; i < r.size(); ++i) r[i] = std::tolower(r[i]);
    return r;
}

std::string NetworkManager::trim(const std::string &s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) ++a;
    while (b > a && (s[b-1] == ' ' || s[b-1] == '\t' || s[b-1] == '\r' || s[b-1] == '\n')) --b;
    return s.substr(a, b - a);
}

NetworkManager::ChunkDecodeResult NetworkManager::tryDecodeChunked(ConnState &st, std::string &decodedBody, size_t &totalConsumed)
{
    size_t p = st.chunkParsePos;
    decodedBody.clear();

    while (true) {
        size_t lineEnd = st.buf.find("\r\n", p);
        if (lineEnd == std::string::npos) return CHUNK_NEED_MORE;

        std::string sizeLine = st.buf.substr(p, lineEnd - p);
        sizeLine = trim(sizeLine);

        // Strip chunk extensions: "<hex>;..."
        size_t semi = sizeLine.find(';');
        if (semi != std::string::npos)
            sizeLine.erase(semi);
        sizeLine = trim(sizeLine);

        if (sizeLine.empty())
            return CHUNK_INVALID;

        // Validate hex digits strictly
        for (size_t i = 0; i < sizeLine.size(); ++i) {
            char c = sizeLine[i];
            bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            if (!isHex)
                return CHUNK_INVALID;
        }

        size_t chunkSize = 0;
        {
            std::istringstream iss(sizeLine);
            iss >> std::hex >> chunkSize;
            if (!iss.eof() && iss.fail())
                return CHUNK_INVALID;
        }

        // Debug: show where we are and the parsed chunk size
        std::cerr << "[NM][chunked] p=" << p << " lineEnd=" << lineEnd
                  << " sizeLine='" << sizeLine << "' chunkSize=" << chunkSize
                  << " buf.size=" << st.buf.size() << std::endl;

        p = lineEnd + 2;

        if (chunkSize == 0) {
            // RFC 7230: after last-chunk (0\r\n), there may be trailer-fields, then a final CRLF.
            // The minimal valid ending is "0\r\n\r\n" (no trailers).
            if (st.buf.size() < p + 2)
                return CHUNK_NEED_MORE;

            // If trailers are empty, we should see CRLF immediately.
            if (st.buf.compare(p, 2, "\r\n") == 0) {
                totalConsumed = p + 2;
                std::cerr << "[NM][chunked] end (no trailers) totalConsumed=" << totalConsumed << std::endl;
                return CHUNK_OK;
            }

            // Otherwise, read trailers until CRLFCRLF.
            size_t trailerEnd = st.buf.find("\r\n\r\n", p);
            std::cerr << "[NM][chunked] chunkSize=0 trailer search from p=" << p
                      << " trailerEnd=" << (trailerEnd == std::string::npos ? -1 : (long long)trailerEnd)
                      << " buf.size=" << st.buf.size() << std::endl;
            if (trailerEnd == std::string::npos)
                return CHUNK_NEED_MORE;
            totalConsumed = trailerEnd + 4;
            std::cerr << "[NM][chunked] end (with trailers) totalConsumed=" << totalConsumed << std::endl;
            return CHUNK_OK;
        }

        // Prevent decoded body from growing without bound
        if (decodedBody.size() + chunkSize > kMaxBodyBytes)
            return CHUNK_INVALID;

        if (st.buf.size() < p + chunkSize + 2) return CHUNK_NEED_MORE;

        decodedBody.append(st.buf, p, chunkSize);
        p += chunkSize;

        if (st.buf.substr(p, 2) != "\r\n") return CHUNK_INVALID;
        p += 2;
    }
}
