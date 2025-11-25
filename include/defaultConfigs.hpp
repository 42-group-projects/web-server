// This header provides default configuration values for the server.
// If mandatory directives are missing in the config file, the server will refuse to start.
// For optional directives, these defaults will be used as fallback values.

// このヘッダはサーバのデフォルト設定値を定義します。
// 設定ファイルに必須ディレクティブが欠けている場合、サーバは起動しません。
// 任意ディレクティブについては、これらのデフォルト値がフォールバックとして使用されます。

#pragma once

//Default configuration file location
// uncomment out ../configFiles/test.conf if you want to run the tester.
#define DEFAULT_CONF_FILE "../configFiles/test.conf"

//Server
#define SERVER_NAME "_"
#define CLIENT_MAX_BODY_SIZE 1024 * 1024 // 1 MB

//Http limits
#define MAX_HEADERS_SIZE 1024 * 8 // 8 KB
#define MAX_HEADERS_COUNT 100 //this can be flexable based on your needs
#define MAX_PAYLOAD_SIZE 1024 * 50 // this is also flexable, set to 50KB for now
#define MAX_REQUEST_LINE_LENGTH 1024 * 2 // this is also flexable

//Parser
#define CONFIG_FILE_MAX_TOKEN_SIZE 200
#define CONFIG_FILE_MAX_CLIENT_BODY_SIZE 2UL * 1024 * 1024 * 1024 // 2GB (To keep users from setting crazy numbers)

//Location specific
#define GET_ALLOWED      true
#define POST_ALLOWED     false
#define DELETE_ALLOWED   false
#define INDEX            ""
#define AUTOINDEX        false
#define REDIRECT_ENABLED false
#define REDIRECT_URL     ""
#define UPLOAD_ENABLED   false
#define UPLOAD_STORE     ""
