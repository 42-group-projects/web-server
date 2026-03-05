#include "NetworkManager.hpp"
#include "../http/HttpHandler/HttpHandler.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"
#include <sys/wait.h>

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

// ============================================================================
// Async CGI Pipe Handlers
// ============================================================================

void NetworkManager::handleCgiPipeRead(int pipeFd)
{
    std::map<int, PendingCgiProcess>::iterator it = pendingCgiProcesses.find(pipeFd);
    if (it == pendingCgiProcesses.end()) return;

    PendingCgiProcess &proc = it->second;
    char buf[4096];
    ssize_t n = read(pipeFd, buf, sizeof(buf));

    if (n > 0) {
        proc.output.append(buf, n);
        if (proc.output.size() > kMaxBodyBytes) {
            std::cerr << "[CGI] output too large, killing pid=" << proc.pid << std::endl;
            kill(proc.pid, SIGKILL);
            completeCgiProcess(pipeFd);
        }
        return;
    }
    if (n == 0) {
        // EOF — CGI finished writing
        completeCgiProcess(pipeFd);
        return;
    }
    // n < 0: EAGAIN/EWOULDBLOCK, nothing ready yet
}

void NetworkManager::completeCgiProcess(int pipeFd)
{
    std::map<int, PendingCgiProcess>::iterator it = pendingCgiProcesses.find(pipeFd);
    if (it == pendingCgiProcesses.end()) return;

    PendingCgiProcess proc = it->second;  // copy before erase
    int clientFd = proc.clientFd;

    // Remove pipe from poll and close it
    for (std::vector<struct pollfd>::iterator pi = pollfds.begin(); pi != pollfds.end(); ++pi) {
        if (pi->fd == pipeFd) { pollfds.erase(pi); break; }
    }
    close(pipeFd);

    // Reap child
    int wstatus;
    waitpid(proc.pid, &wstatus, 0);

    // Build response from CGI output
    HttpResponse res;
    res.parseCgiResponse(proc.output);

    if (res.getVersion().empty() || res.getVersion().find("HTTP/") != 0)
        res.setVersion("HTTP/1.1");
    if (res.getStatus() == UNSET) {
        res.setStatus(INTERNAL_SERVER_ERROR);
        if (res.getMimeType().empty()) res.setMimeType("text/plain");
        if (res.getBody().empty()) res.setBody("Internal Server Error\n");
    }
    res.setHeader("Server", "webserv");
    res.setHeader("Host", getServerName(clientPorts[clientFd]));

    // Keep-alive decision
    bool keep = shouldKeepAlive(proc.requestHead);
    std::map<int, ConnState>::iterator si = states.find(clientFd);
    if (si != states.end()) {
        si->second.requestCount++;
        if (si->second.requestCount >= kMaxRequestsPerConnection)
            keep = false;
        si->second.wantClose = !keep;
    }
    res.setHeader("Connection", keep ? "keep-alive" : "close");

    // Queue response and switch client to write mode
    sendBufs[clientFd] = res.generateResponse(res.getStatus());
    enableWriteMode(clientFd);

    pendingCgiProcesses.erase(it);
    std::cerr << "[CGI] completed for client fd=" << clientFd << std::endl;
}

void NetworkManager::cleanupTimedOutCgiProcesses()
{
    time_t now = time(NULL);
    std::vector<int> timedOut;

    for (std::map<int, PendingCgiProcess>::iterator it = pendingCgiProcesses.begin();
         it != pendingCgiProcesses.end(); ++it) {
        if (now - it->second.startTime > kCgiTimeoutSec)
            timedOut.push_back(it->first);
    }

    for (size_t i = 0; i < timedOut.size(); ++i) {
        int pipeFd = timedOut[i];
        PendingCgiProcess &proc = pendingCgiProcesses[pipeFd];
        std::cerr << "[CGI] timeout pid=" << proc.pid << " client fd=" << proc.clientFd << std::endl;
        kill(proc.pid, SIGKILL);
        sendErrorResponse(proc.clientFd, GATEWAY_TIMEOUT, "CGI Timeout\n", proc.requestHead, true);

        for (std::vector<struct pollfd>::iterator pi = pollfds.begin(); pi != pollfds.end(); ++pi) {
            if (pi->fd == pipeFd) { pollfds.erase(pi); break; }
        }
        close(pipeFd);
        waitpid(proc.pid, NULL, WNOHANG);
        pendingCgiProcesses.erase(pipeFd);
    }
}
