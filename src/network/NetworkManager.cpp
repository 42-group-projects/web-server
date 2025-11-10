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
    // close fd and erase from containers
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
        return;
    }
    ConnState &st = states[fd];
    if (!st.headerDone && st.contentLength == 0 && st.isChunked == false && st.headerEndPos == 0) {
        // 初期化（未登録時のデフォルト）
        st.headerDone = false;
        st.headerEndPos = 0;
        st.isChunked = false;
        st.contentLength = (size_t)-1;
        st.chunkParsePos = 0;
    }
    st.buf.append(buf, n);

    // 1回の recv で複数リクエストを処理する可能性があるためループ
    while (tryParseRequest(fd)) {
        // 完了ごとに次のリクエスト検出に挑戦
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

// 受信バッファから 1 つの完全なリクエストを検出し、発見時には簡易 200 をキューする
bool NetworkManager::tryParseRequest(int fd)
{
    ConnState &st = states[fd];

    // ヘッダ未解析なら、ヘッダ終端を探す
    if (!st.headerDone) {
        size_t pos = st.buf.find("\r\n\r\n");
        if (pos == std::string::npos) return false; // ヘッダ未完
        st.headerDone = true;
        st.headerEndPos = pos + 4; // 本文開始位置
        parseHeadersAndInitState(st);
    }

    // 本文の完了判定
    bool complete = false;
    size_t totalConsumed = 0; // リクエスト全体で消費する長さ

    if (st.isChunked) {
        std::string decoded;
        if (!tryDecodeChunked(st, decoded, totalConsumed)) {
            return false; // まだ足りない
        }
        // ヘッダ部分を使って HttpRequest をパースし、ボディはアンチャンク済みをセット
        std::string head = st.buf.substr(0, st.headerEndPos);
        HttpRequest req;
        // ヘッダのみでパースを走らせる（ボディは後からセット）
        try { req.parseRequest(head); } catch(...) {}
        req.setBody(decoded);
        // Content-Length を正しく設定（上書き）
        {
            std::ostringstream cl; cl << decoded.size();
            req.setHeader("Content-Length", cl.str());
        }
        // ハンドラ呼び出し
        HttpHandler handler;
        HttpResponse res = handler.handleRequest(req, config);
        // バージョンのフォールバック（不正な既定値 "Http1.1" を避ける）
        if (res.getVersion().empty() || res.getVersion().find("HTTP/") != 0) {
            res.setVersion("HTTP/1.1");
        }
        // ステータス未設定のフォールバック
        if (res.getStatus() == UNSET) {
            res.setStatus(INTERNAL_SERVER_ERROR);
            if (res.getMimeType().empty()) res.setMimeType("text/plain");
            if (res.getBody().empty()) res.setBody("Internal Server Error\n");
        }
        // 必要なら接続を閉じる
        res.setHeader("Connection", "close");
        // httpTester の流儀に合わせて generateResponse を使用
        std::string out = res.generateResponse(res.getStatus());
        sendBufs[fd] = out;
        // POLLOUT 有効化
        for (size_t i = 0; i < pollfds.size(); ++i) if (pollfds[i].fd == fd) { pollfds[i].events |= POLLOUT; break; }
        complete = true;
    } else {
        size_t bodyHave = (st.buf.size() >= st.headerEndPos) ? (st.buf.size() - st.headerEndPos) : 0;
        size_t need = (st.contentLength == (size_t)-1) ? 0 : st.contentLength;
        if (bodyHave >= need) {
            totalConsumed = st.headerEndPos + need;
            // ヘッダとボディを抽出し、HttpHandler へ渡す
            std::string head = st.buf.substr(0, st.headerEndPos);
            std::string body = (need == 0) ? std::string() : st.buf.substr(st.headerEndPos, need);

            HttpRequest req;
            try { req.parseRequest(head); } catch(...) {}
            if (!body.empty()) {
                req.setBody(body);
                std::ostringstream cl; cl << body.size();
                req.setHeader("Content-Length", cl.str());
            }

            HttpHandler handler;
            HttpResponse res = handler.handleRequest(req, config);
            if (res.getVersion().empty() || res.getVersion().find("HTTP/") != 0) {
                res.setVersion("HTTP/1.1");
            }
            if (res.getStatus() == UNSET) {
                res.setStatus(INTERNAL_SERVER_ERROR);
                if (res.getMimeType().empty()) res.setMimeType("text/plain");
                if (res.getBody().empty()) res.setBody("Internal Server Error\n");
            }
            res.setHeader("Connection", "close");
            std::string out = res.generateResponse(res.getStatus());
            sendBufs[fd] = out;
            for (size_t i = 0; i < pollfds.size(); ++i) if (pollfds[i].fd == fd) { pollfds[i].events |= POLLOUT; break; }
            complete = true;
        }
    }

    if (!complete) return false;

    // 読み込みを無効化（今回の最小実装では close 予定）
    for (size_t i = 0; i < pollfds.size(); ++i) {
        if (pollfds[i].fd == fd) { pollfds[i].events &= ~POLLIN; break; }
    }

    // 消費した分をバッファから削除（パイプライン対応）
    if (totalConsumed > 0 && totalConsumed <= st.buf.size())
        st.buf.erase(0, totalConsumed);
    // 次のリクエストに備えて状態リセット
    st.headerDone = false; st.headerEndPos = 0; st.isChunked = false; st.contentLength = (size_t)-1; st.chunkParsePos = 0;
    return true;
}

static std::map<std::string, std::string> parseHeaderMap(const std::string &headersRaw)
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
    // リクエストライン + ヘッダ部分を抽出
    if (st.headerEndPos > st.buf.size()) return; // safety
    std::string head = st.buf.substr(0, st.headerEndPos);

    // ヘッダ部分のみを抽出（最初の行を含んでいても parseHeaderMap は ':' のある行だけ拾う）
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

// st.buf の st.chunkParsePos から chunked 本文を解析し、
// 完全に揃っていれば decodedBody と合計消費バイト totalConsumed を返す
bool NetworkManager::tryDecodeChunked(ConnState &st, std::string &decodedBody, size_t &totalConsumed)
{
    size_t p = st.chunkParsePos;
    decodedBody.clear();
    while (true) {
        // サイズ行を探す
        size_t lineEnd = st.buf.find("\r\n", p);
        if (lineEnd == std::string::npos) return false; // サイズ行未完
        std::string sizeHex = st.buf.substr(p, lineEnd - p);
        sizeHex = trim(sizeHex);
        // 16進サイズを読む
        size_t chunkSize = 0;
        {
            std::istringstream iss(sizeHex);
            iss >> std::hex >> chunkSize;
            if (!iss.good() && !iss.eof()) return false; // 異常
        }
        p = lineEnd + 2; // データ開始位置
        if (chunkSize == 0) {
            // 終了チャンク。後続の CRLF を要求
            if (st.buf.size() < p + 2) return false; // 末尾 CRLF 未着
            if (st.buf.substr(p, 2) != "\r\n") return false; // 形式異常（保守）
            p += 2;
            totalConsumed = p; // ヘッダからの相対でなく、st.buf 先頭からの消費長
            return true;
        }
        // チャンクデータ本体
        if (st.buf.size() < p + chunkSize + 2) return false; // 未着
        decodedBody.append(st.buf, p, chunkSize);
        p += chunkSize;
        // チャンク末尾 CRLF
        if (st.buf.substr(p, 2) != "\r\n") return false; // 形式異常（保守）
        p += 2;
        // 次のチャンクサイズ行へ
    }
}
