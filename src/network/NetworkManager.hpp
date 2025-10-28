#pragma once

#include "../../include/imports.hpp"
#include "../../include/globals.hpp"

#include <poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cerrno>
#include <cstring>
#include <map>

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    // 初期化: g_config.getListen() を使って複数ポートを listen
    bool init();

    // イベントループ開始（ブロッキング）
    void run();

    // 停止要求
    void stop();

private:
    bool running;

    std::vector<int> listeners;                 // リスニングFD一覧
    std::vector<struct pollfd> pollfds;         // 監視対象（リスナー + クライアント）
    std::map<int, bool> isListener;             // fd がリスナーかどうか
    std::map<int, std::string> sendBufs;        // クライアント送信バッファ

    struct ConnState {
        std::string buf;        // 受信蓄積バッファ（未処理分）
        bool headerDone;        // ヘッダ終端検出済み
        size_t headerEndPos;    // "\r\n\r\n" の直後インデックス
        bool isChunked;         // Transfer-Encoding: chunked
        size_t contentLength;   // Content-Length（なければ (size_t)-1）
        // chunked 解析用一時状態
        size_t chunkParsePos;   // 次のチャンクサイズ行の開始位置（buf 内）
    };

    std::map<int, ConnState> states;             // 各接続の状態

    // 内部ユーティリティ
    bool addListener(const std::string &ip, int port);
    void addPollFd(int fd, short events, bool listener);
    void removeFd(int fd);
    bool setNonBlocking(int fd);

    // I/O ハンドラ
    void handleListenerEvent(int fd);
    void handleClientRead(int fd);
    void handleClientWrite(int fd);
    void queueSimpleOkResponse(int fd); // 最小テスト用の固定レスポンス

    // パース補助
    bool tryParseRequest(int fd); // 完全なリクエストを1つ消化したら true（ループで複数処理）
    void parseHeadersAndInitState(ConnState &st);
    static std::string toLower(const std::string &s);
    static std::string trim(const std::string &s);
    bool tryDecodeChunked(ConnState &st, std::string &decodedBody, size_t &totalConsumed);
};
