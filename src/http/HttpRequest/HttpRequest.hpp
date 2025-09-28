
#include "../../../include/imports.hpp"
#include "../include/enums.hpp"

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

class HttpRequest
{
public:
	HttpRequest();
	HttpRequest(const std::string &raw_request);
	HttpRequest &operator=(const HttpRequest &other);
	~HttpRequest();
	void parseRequest(const std::string &request);
	void displayRequest() const;

private:
	e_method method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	void parse_request_line(std::istringstream& line_stream);
	void parse_headers(std::istringstream& line_stream);
	// void parse_body(std::istringstream& line_stream);
	// Add other necessary members;
};

#endif
