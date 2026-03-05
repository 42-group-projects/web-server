#include "NetworkManager.hpp"

// ============================================================================
// Connection Lifecycle Management
// ============================================================================

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
    if (!parseIPv4ToNetworkOrder(ip, addr.sin_addr.s_addr)) {
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

        const std::string clientIp = formatIPv4FromNetworkOrder(cli.sin_addr.s_addr);
        clientIps[cfd] = clientIp;
        struct sockaddr_in localAddr;
        socklen_t addrlen = sizeof(localAddr);
        if (getsockname(cfd, (struct sockaddr*)&localAddr, &addrlen) == 0)
        {
            int serverPort = ntohs(localAddr.sin_port);
            clientPorts[cfd] = serverPort;
        }
        std::cout
            << "Accepted fd=" << cfd
            << " from " << clientIp << ":" << ntohs(cli.sin_port)
            << " on port " << clientPorts[cfd] << std::endl;
    }
}
