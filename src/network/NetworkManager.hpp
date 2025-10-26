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
    std::map<int, std::string> recvBufs;        // クライアント受信バッファ
    std::map<int, std::string> sendBufs;        // クライアント送信バッファ

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
};
