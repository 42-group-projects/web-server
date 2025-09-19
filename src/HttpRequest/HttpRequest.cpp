#include "./HttpRequest.hpp"

HttpRequest::HttpRequest() : method(""), uri(""), version("") 
{
    std::cout << "HttpRequest constructor called" << std::endl;
}
HttpRequest::~HttpRequest() 
{
    std::cout << "HttpRequest destructor called" << std::endl;
}
void HttpRequest::parseRequest(const std::string &request) 
{
    // size_t method_end = request.find(' ');
    // if (method_end == std::string::npos) return;
    // method = request.substr(0, method_end);

    // size_t uri_start = method_end + 1;
    // size_t uri_end = request.find(' ', uri_start);
    // if (uri_end == std::string::npos) return;
    // uri = request.substr(uri_start, uri_end - uri_start);

    // size_t version_start = uri_end + 1;
    // size_t version_end = request.find("\r\n", version_start);
    // if (version_end == std::string::npos) return;
    // version = request.substr(version_start, version_end - version_start);

    // to avoid unused parameter warning
    (void)request;

    method = "GET";
    uri = "/index.html";
    version = "HTTP/1.1";
    std::cout << "Method: " << method << "\nURI: " << uri << "\nVersion: " << version << std::endl;
}