#include "HttpRequest.hpp"

HttpRequest::HttpRequest() : method(UNDEFINED), uri(""), version("") 
{
    std::cout << "HttpRequest constructor called" << std::endl;
}
HttpRequest::~HttpRequest() 
{
    std::cout << "HttpRequest destructor called" << std::endl;
}
void HttpRequest::parseRequest(const std::string &request) 
{
    // This is just to show that the function is called
    (void)request;

    method = GET;
    uri = "/index.html";
    version = "HTTP/1.1";
    this->displayRequest();
}

void HttpRequest::displayRequest() const
{
    std::cout << "Method: " << method << "\nURI: " << uri << "\nVersion: " << version << std::endl;
}
