#include "NetworkManager.hpp"

// ============================================================================
// NetworkManager Lifecycle: Constructor, Destructor, Init, Stop, Event Loop
// ============================================================================

NetworkManager::NetworkManager(const ServerConfig& config) : config(config), running(false) {}

NetworkManager::~NetworkManager() {
    for (size_t i = 0; i < pollfds.size(); ++i) {
        close(pollfds[i].fd);
    }
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
