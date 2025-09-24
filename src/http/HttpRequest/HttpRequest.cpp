#include "HttpRequest.hpp"
#include "../utils.hpp"
#include "../../../include/imports.hpp"

HttpRequest::HttpRequest() : method(UNDEFINED), uri(""), version("")
{
    std::cout << "HttpRequest constructor called" << std::endl;
}

HttpRequest::HttpRequest(const std::string &raw_request) : method(UNDEFINED), uri(""), version("")
{
    std::cout << "HttpRequest parameterized constructor called" << std::endl;
    this->parseRequest(raw_request);
}

HttpRequest &HttpRequest::operator=(const HttpRequest &other)
{
    if (this != &other)
    {
        this->method = other.method;
        this->uri = other.uri;
        this->version = other.version;
        this->headers = other.headers;
        this->body = other.body;
        // Copy other necessary members
    }
    return *this;
}

HttpRequest::~HttpRequest()
{
    std::cout << "HttpRequest destructor called" << std::endl;
}

void HttpRequest::parseRequest(const std::string &request)
{
    std::istringstream request_stream(request);
    std::string line;

    // Parse request line
    if (std::getline(request_stream, line))
    {
        if (line.empty())
            throw std::runtime_error("Empty request line encountered");
        std::istringstream line_stream(line);
        parse_request_line(line_stream);
    }

    // Parse headers
    while (std::getline(request_stream, line) && line != "\r")
    {
        if (line.empty())
            throw std::runtime_error("Empty header line encountered");
        std::istringstream line_stream(line);
        parse_headers(line_stream);
    }

    // Parse body
    while (std::getline(request_stream, line))
    {
        body += line + "\n";
    }
    // if (!body.empty() && body.back() == '\n')
        // body.pop_back(); // Remove the last newline character
}


void HttpRequest::parse_request_line(std::istringstream& line_stream)
{
    std::string method_str;
    if (line_stream >> method_str >> uri >> version)
    {
        if (method_str == "GET")
            method = GET;
        else if (method_str == "POST")
            method = POST;
        else if (method_str == "DELETE")
            method = DELETE;
        else
            method = UNDEFINED;
    }
}

void HttpRequest::parse_headers(std::istringstream& line_stream)
{
    std::string key, value;
    if (std::getline(line_stream, key, ':') && std::getline(line_stream, value))
    {
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        if(key.empty() || value.empty())
            throw std::runtime_error("Malformed header line: key or value is empty");
        headers[key] = value;
    }
}

void HttpRequest::displayRequest() const
{   
    std::cout << "-------HTTP REQUEST-------" << std::endl;
    std::cout << "Method: " << get_method_string(method)
              << "\nURI: " << uri
              << "\nVersion: " << version
              << std::endl;

    std::cout << "-----------Headers-----------" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "-----------Body-----------" << std::endl;
    std::cout << body << std::endl;
}
