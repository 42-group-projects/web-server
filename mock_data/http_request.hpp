#ifndef HTTP_REQUEST_MOCK_DATA_HPP
    #define HTTP_REQUEST_MOCK_DATA_HPP
#include <string>

//GET request
const std::string simple_get_request =
    "GET /index.html HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\r\n";

const std::string simple_broken_get_request =
    "GET /index.html HTTP/1.1\n"
    "Host: www.example.com\n"
    "User-Agent: Mozilla/5.0\n"
    "Accept: text/html\n"
    "\n";

const std::string simple_missing_request_line_request =
    "Host: www.example.com\n"
    "User-Agent: Mozilla/5.0\n"
    "Accept: text/html\n"
    "\n";

const std::string get_request_with_params =
    "GET /search?q=test&lang=en HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Connection: keep-alive\r\n"
    "Accept: text/html\r\n"
    "\r\n";

//POST request
const std::string simple_post_request =
    "POST /submit-form HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "Content-Length: 38\r\n"
    "\r\n"
    "username=johndoe&age=30&password=12345";

const std::string post_request_with_json =
    "POST /api/data HTTP/1.1\r\n"
    "Host: api.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 54\r\n"
    "\r\n"
    "{\"name\":\"John Doe\",\"email\":\"john@example.com\"}";

//Delete request
const std:: string simple_delete_request =
    "DELETE /api/resource/123 HTTP/1.1\r\n"
    "Host: api.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Authorization: Bearer 12bk1naiu9f\r\n"
    "\r\n";

//BAD REQUEST
const std::string bad_request_missing_http_version =
    "GET /index.html\r\n" // Missing HTTP/1.1
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\r\n";

const std:: string bad_request_invalid_method =
    "FETCH /index.html HTTP/1.1\r\n" // Fetch is not a valid HTTP method
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\r\n";

const std::string bad_request_malformed_headers =
    "GET /index.html HTTP/1.1\r\n"
    "Host www.example.com\r\n" // Missing colon
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\r\n";

#endif
