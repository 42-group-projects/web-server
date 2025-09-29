// This header provides default configuration values for the server.
// If mandatory directives are missing in the config file, the server will refuse to start.
// For optional directives, these defaults will be used as fallback values.

// このヘッダはサーバのデフォルト設定値を定義します。
// 設定ファイルに必須ディレクティブが欠けている場合、サーバは起動しません。
// 任意ディレクティブについては、これらのデフォルト値がフォールバックとして使用されます。

#pragma once

//Default configuration file location
#define DEFAULT_CONF_FILE "configFiles/webserv.conf"

//Server
#define SERVER_NAME_DEFAULT "mySite.local"
#define CLIENT_MAX_BODY_SIZE_DEFAULT 1024 * 1024 // 1 MB

//Location specific
#define GET_ALLOWED_DEFAULT         true
#define POST_ALLOWED_DEFAULT        false
#define DELETE_ALLOWED_DEFAULT      false
#define INDEX_DEFAULT               "index.html"
#define AUTOINDEX_DEFAULT           false
#define REDIRECT_ENABLED_DEFAULT    false
#define REDIRECT_URL_DEFAULT        ""
#define UPLOAD_ENABLED_URL_DEFAULT  false
#define UPLOAD_STORE_DEFAULT        ""
