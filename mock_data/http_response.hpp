#ifndef HTTP_RESPONSE_MOCK_DATA_HPP
    #define HTTP_RESPONSE_MOCK_DATA_HPP
#include <string>

//GET response
const std::string simple_get_response = 
    "HTTP/1.1 200 OK\r\n"
    "Server: WebServer/1.0\r\n"
    "Date: Mon, 27 Jul 2023 12:28:53 GMT\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 130\r\n"
    "\r\n"
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<head>\r\n"
    "    <title>Example Page</title>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "    <h1>Hello World!</h1>\r\n"
    "</body>\r\n"
    "</html>";

const std::string get_with_params_response = 
    "HTTP/1.1 200 OK\r\n"
    "Server: WebServer/1.0\r\n"
    "Date: Mon, 27 Jul 2023 12:30:15 GMT\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 184\r\n"
    "\r\n"
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<head>\r\n"
    "    <title>Search Results</title>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "    <h1>Search Results for 'test'</h1>\r\n"
    "    <p>Language: en</p>\r\n"
    "</body>\r\n"
    "</html>";

//POST response
const std::string simple_post_response = 
    "HTTP/1.1 201 Created\r\n"
    "Server: WebServer/1.0\r\n"
    "Date: Mon, 27 Jul 2023 12:35:42 GMT\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 39\r\n"
    "\r\n"
    "Form submitted successfully. Thank you!";

const std::string post_response_json = 
    "HTTP/1.1 200 OK\r\n"
    "Server: WebServer/1.0\r\n"
    "Date: Mon, 27 Jul 2023 12:40:21 GMT\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 97\r\n"
    "\r\n"
    "{\"status\":\"success\",\"message\":\"Data received\",\"user\":{\"id\":42,\"name\":\"John Doe\"}}";

//DELETE response
const std::string delete_response = 
    "HTTP/1.1 200 OK\r\n"
    "Server: WebServer/1.0\r\n"
    "Date: Mon, 27 Jul 2023 12:45:30 GMT\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 70\r\n"
    "\r\n"
    "{\"status\":\"success\",\"message\":\"Resource successfully deleted\"}";

//BAD RESPONSE
const std::string bad_request_400 = 
    "HTTP/1.1 400 Bad Request\r\n"
    "Server: WebServer/1.0\r\n"
    "Date: Mon, 27 Jul 2023 12:50:10 GMT\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 45\r\n"
    "\r\n"
    "400 Bad Request: Malformed request syntax.";

const std::string not_found_404 = 
    "HTTP/1.1 404 Not Found\r\n"
    "Server: WebServer/1.0\r\n"
    "Date: Mon, 27 Jul 2023 12:55:05 GMT\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 144\r\n"
    "\r\n"
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<head>\r\n"
    "    <title>404 Not Found</title>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "    <h1>404 Not Found</h1>\r\n"
    "</body>\r\n"
    "</html>";

#endif
