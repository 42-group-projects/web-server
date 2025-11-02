// This header provides default configuration values for the server.
// If mandatory directives are missing in the config file, the server will refuse to start.
// For optional directives, these defaults will be used as fallback values.

// このヘッダはサーバのデフォルト設定値を定義します。
// 設定ファイルに必須ディレクティブが欠けている場合、サーバは起動しません。
// 任意ディレクティブについては、これらのデフォルト値がフォールバックとして使用されます。

#pragma once

//Default configuration file location
#define DEFAULT_CONF_FILE "../configFiles/minoka.conf"

//Server
#define SERVER_NAME "mySite.local"
#define CLIENT_MAX_BODY_SIZE 1024 * 1024 // 1 MB

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
