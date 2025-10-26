#include "NetworkManager.hpp"

NetworkManager::NetworkManager() : running(false) {}
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
    // close fd and erase from containers
    close(fd);
    for (std::vector<struct pollfd>::iterator it = pollfds.begin(); it != pollfds.end(); ++it) {
        if (it->fd == fd) { pollfds.erase(it); break; }
    }
    isListener.erase(fd);
    recvBufs.erase(fd);
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
    const std::vector<std::pair<std::string, int> > &lst = g_config.getListen();
    if (lst.empty()) {
        std::cerr << "No listen directives found in config." << std::endl;
        return false;
    }

    bool anyFailure = false;
    for (size_t i = 0; i < lst.size(); ++i) {
        if (!addListener(lst[i].first, lst[i].second)) {
            std::cerr << "Failed to add listener for " << lst[i].first << ":" << lst[i].second << std::endl;
            anyFailure = true; // try others but mark failure
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

void NetworkManager::handleListenerEvent(int fd)
{
    // accept できる分受け入れる（errno チェックは禁止なので、-1 が来たらそこで止める）
    while (true) {
        struct sockaddr_in cli; socklen_t len = sizeof(cli);
        int cfd = accept(fd, (struct sockaddr*)&cli, &len);
        if (cfd < 0) {
            break;
        }
        if (!setNonBlocking(cfd)) { close(cfd); continue; }
        addPollFd(cfd, POLLIN, false);
        char ipbuf[INET_ADDRSTRLEN];
        const char *p = inet_ntop(AF_INET, &cli.sin_addr, ipbuf, sizeof(ipbuf));
        std::cout << "Accepted fd=" << cfd << " from " << (p ? p : "?") << ":" << ntohs(cli.sin_port) << std::endl;
    }
}

void NetworkManager::queueSimpleOkResponse(int fd)
{
    static const char body[] = "Hello from webserv\n";
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: text/plain\r\n";
    oss << "Content-Length: " << sizeof(body) - 1 << "\r\n";
    oss << "Connection: close\r\n\r\n";
    std::string header = oss.str();
    sendBufs[fd] = header + std::string(body, sizeof(body) - 1);

    // POLLOUT を有効化
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
    if (n <= 0) {
        // 切断または一時的に読み取れない -> 次回へ（エラー判定は poll のイベントで行う）
        // クライアントが切断している可能性があるので、POLLHUP/POLLERR に任せる。
        return;
    }
    recvBufs[fd].append(buf, n);

    // 最小動作: ヘッダ終端を検出したら固定レスポンスを送る
    if (recvBufs[fd].find("\r\n\r\n") != std::string::npos) {
        queueSimpleOkResponse(fd);
        // 次の読み込みは不要なので READ を無効化（close は送信完了後）
        for (size_t i = 0; i < pollfds.size(); ++i) {
            if (pollfds[i].fd == fd) {
                pollfds[i].events &= ~POLLIN;
                break;
            }
        }
    }
}

void NetworkManager::handleClientWrite(int fd)
{
    std::map<int, std::string>::iterator it = sendBufs.find(fd);
    if (it == sendBufs.end()) {
        // 送るものがない -> 書き込み監視を外す
        for (size_t i = 0; i < pollfds.size(); ++i) {
            if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLOUT; break; }
        }
        return;
    }

    const std::string &data = it->second;
    ssize_t n = send(fd, data.data(), data.size(), 0);
    if (n <= 0) {
        // 一時的に送れない or エラー -> 次回へ（POLLERR 等に任せる）
        return;
    }

    if ((size_t)n >= data.size()) {
        // 全部送れた -> 接続を閉じる（最小実装では Connection: close）
        removeFd(fd);
    } else {
        // まだ残りがある -> 残りを更新
        it->second.erase(0, n);
    }
}

void NetworkManager::run()
{
    if (!running) return;
    std::cout << "Network loop started." << std::endl;
    while (running) {
        int ret = poll(&pollfds[0], pollfds.size(), 1000);
        if (ret < 0) {
            // poll 失敗 -> ループ継続
            continue;
        }
        if (ret == 0) {
            // timeout
            continue;
        }

        // revents を先にコピー（removeFd で配列が変化するのを避けるため）
        std::vector<struct pollfd> current = pollfds;
        for (size_t i = 0; i < current.size(); ++i) {
            int fd = current[i].fd;
            short re = current[i].revents;
            if (re == 0) continue;

            // エラー/切断は優先的に処理
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
