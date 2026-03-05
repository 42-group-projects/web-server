#include "NetworkManager.hpp"
#include "../http/HttpHandler/HttpHandler.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"

// ============================================================================
// I/O Event Handlers (Read/Write)
// ============================================================================

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
        // Non-blocking socket: negative return means no data available yet
        // or a transient error - just return and wait for next poll event
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
        // Non-blocking socket: negative return means cannot send yet
        // or a transient error - just return and wait for next poll event
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
                // Non-blocking socket: no more data available right now
                break;
            }
        }

        // Now parse any buffered requests.
        while (tryParseRequest(fd)) {
        }

        return;
    }

    it->second.erase(0, n);
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
