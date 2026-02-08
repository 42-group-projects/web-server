#pragma once

#include "../../include/imports.hpp"
#include "../configFileParser/ServerConfig.hpp"

#include <poll.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cerrno>
#include <cstring>
#include <map>

class NetworkManager {
public:
    NetworkManager(const ServerConfig& config);
    ~NetworkManager();

    // 初期化: g_config.getListen() を使って複数ポートを listen
    // Now it's config.getAllListen() - Clément
    bool init();

    // イベントループ開始（ブロッキング）
    void run();

    // 停止要求
    void stop();

private:
    const ServerConfig& config;
    bool running;

    //=====================================
    std::map<int, std::string> clientIps;
    std::map<int, int> clientPorts;
    //==============================Clement

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

        // keep-alive / connection management
        bool wantClose;         // このレスポンス送信後に接続を閉じるか
        size_t requestCount;    // この接続で処理したリクエスト数
        time_t lastActivity;    // 最終I/O時刻（timeout判定用）
    };

    std::map<int, ConnState> states;             // 各接続の状態

    // chunked decode result to distinguish “need more” vs “bad request”
    enum ChunkDecodeResult {
        CHUNK_OK = 0,
        CHUNK_NEED_MORE,
        CHUNK_INVALID
    };

    // hard limits (DoS / memory protection)
    static const size_t kMaxHeaderBytes = 32 * 1024;        // 32KB
    static const size_t kMaxBodyBytes = 10 * 1024 * 1024;   // 10MB

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
    static std::map<std::string, std::string> parseHeaderMap(const std::string &headersRaw);
    static std::string toLower(const std::string &s);
    static std::string trim(const std::string &s);
    ChunkDecodeResult tryDecodeChunked(ConnState &st, std::string &decodedBody, size_t &totalConsumed);

    // keep-alive policy
    static const int kKeepAliveTimeoutSec = 60;
    static const size_t kMaxRequestsPerConnection = 100;

    void touchActivity(int fd);
    void cleanupIdleConnections();
    bool shouldKeepAlive(const std::string &requestHead) const;
    bool isBodyLengthRequiredError(const std::string &requestHead) const;
};
