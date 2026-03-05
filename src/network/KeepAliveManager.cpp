#include "NetworkManager.hpp"

// ============================================================================
// Keep-Alive and Connection Timeout Management
// ============================================================================

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
