#include "NetworkManager.hpp"
#include "../http/HttpHandler/HttpHandler.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"

// ============================================================================
// Main request parser
// ============================================================================

bool NetworkManager::tryParseRequest(int fd)
{
    ConnState &st = states[fd];

    if (sendBufs.find(fd) != sendBufs.end())
        return false;

    // --- Header phase ---
    if (!st.headerDone) {
        if (st.buf.size() > kMaxHeaderBytes) {
            sendErrorResponse(fd, REQUEST_HEADER_FIELDS_TOO_LARGE,
                              "Request Header Fields Too Large\n", "", true);
            st.buf.clear();
            resetParseState(st);
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

    // --- Body phase ---
    bool complete = false;
    size_t totalConsumed = 0;
    std::string requestHead = st.buf.substr(0, st.headerEndPos);

    {
        size_t lineEnd = requestHead.find("\r\n");
        std::string rl = (lineEnd == std::string::npos) ? requestHead : requestHead.substr(0, lineEnd);
        std::cerr << "[NM][fd=" << fd << "] requestLine='" << rl << "'" << std::endl;
    }

    if (isBodyLengthRequiredError(requestHead)) {
        sendErrorResponse(fd, LENGTH_REQUIRED, "Length Required\n", requestHead, false);
        totalConsumed = st.headerEndPos;
        complete = true;
    } else if (st.isChunked) {
        complete = handleChunkedBody(fd, st, requestHead, totalConsumed);
    } else {
        complete = handleRegularBody(fd, st, requestHead, totalConsumed);
    }

    if (!complete) return false;

    // --- Cleanup phase ---
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

    resetParseState(st);
    return true;
}

void NetworkManager::dispatchRequest(int fd, const std::string &requestHead, const std::string &body)
{
    ConnState &st = states[fd];

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
    enableWriteMode(fd);
}

// ============================================================================
// Chunked body handling
// ============================================================================

bool NetworkManager::handleChunkedBody(int fd, ConnState &st, const std::string &requestHead, size_t &totalConsumed)
{
    std::string decoded;
    ChunkDecodeResult r = tryDecodeChunked(st, decoded, totalConsumed);
    std::cerr << "[NM][fd=" << fd << "] chunked decode result=" << (int)r
              << " decoded.size=" << decoded.size() << " totalConsumed=" << totalConsumed
              << " buf.size=" << st.buf.size() << std::endl;

    if (r == CHUNK_NEED_MORE) {
        if (st.buf.size() > kMaxHeaderBytes + kMaxBodyBytes)
            sendErrorResponse(fd, CONTENT_TOO_LARGE, "Payload Too Large\n", requestHead, true);
        return false;
    }

    if (r == CHUNK_INVALID) {
        sendErrorResponse(fd, BAD_REQUEST, "Bad Request\n", requestHead, true);
        totalConsumed = st.headerEndPos;
    } else if (decoded.size() > kMaxBodyBytes) {
        sendErrorResponse(fd, CONTENT_TOO_LARGE, "Payload Too Large\n", requestHead, true);
        totalConsumed = (totalConsumed > 0) ? totalConsumed : st.headerEndPos;
    } else {
        dispatchRequest(fd, requestHead, decoded);
    }

    if (totalConsumed > st.buf.size())
        totalConsumed = st.buf.size();
    return true;
}

// ============================================================================
// Regular (non-chunked) body handling
// ============================================================================

bool NetworkManager::handleRegularBody(int fd, ConnState &st, const std::string &requestHead, size_t &totalConsumed)
{
    size_t bodyHave = (st.buf.size() >= st.headerEndPos) ? (st.buf.size() - st.headerEndPos) : 0;
    size_t need = (st.contentLength == (size_t)-1) ? 0 : st.contentLength;
    std::cerr << "[NM][fd=" << fd << "] non-chunked bodyHave=" << bodyHave << " need=" << need
              << " headerEndPos=" << st.headerEndPos << " buf.size=" << st.buf.size() << std::endl;

    if (bodyHave < need)
        return false;

    totalConsumed = st.headerEndPos + need;
    std::string body = (need == 0) ? std::string() : st.buf.substr(st.headerEndPos, need);
    dispatchRequest(fd, requestHead, body);
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

void NetworkManager::enableWriteMode(int fd)
{
    for (size_t i = 0; i < pollfds.size(); ++i)
        if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; pollfds[i].events |= POLLOUT; break; }
}

void NetworkManager::resetParseState(ConnState &st)
{
    st.headerDone = false;
    st.headerEndPos = 0;
    st.isChunked = false;
    st.contentLength = (size_t)-1;
    st.chunkParsePos = 0;
}

void NetworkManager::sendErrorResponse(int fd, e_status_code status, const std::string &bodyText,
                                       const std::string &requestHead, bool forceClose)
{
    ConnState &st = states[fd];
    HttpResponse res;
    res.setVersion("HTTP/1.1");
    res.setHeader("Server", "webserv");
    res.setHeader("Host", getServerName(clientPorts[fd]));
    res.setStatus(status);
    res.setMimeType("text/plain");
    res.setBody(bodyText);

    if (forceClose) {
        st.wantClose = true;
        res.setHeader("Connection", "close");
    } else {
        bool keep = shouldKeepAlive(requestHead);
        st.requestCount++;
        if (st.requestCount >= kMaxRequestsPerConnection)
            keep = false;
        st.wantClose = !keep;
        res.setHeader("Connection", keep ? "keep-alive" : "close");
    }

    sendBufs[fd] = res.generateResponse(res.getStatus());
    enableWriteMode(fd);
}
