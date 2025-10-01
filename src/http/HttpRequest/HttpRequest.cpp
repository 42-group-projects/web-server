#include "HttpRequest.hpp"
#include "../utils.hpp"
#include "../../../include/imports.hpp"
#include "../../errorHandling/ErrorWarning.hpp"

HttpRequest::HttpRequest() : method(UNDEFINED), uri(""), version(""), parsing_error(""){}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const std::string &raw_request) : method(UNDEFINED), uri(""), version(""), parsing_error("")
{
	//comment out if nessisary
		this->displayRequest();
	try
	{
		this->parseRequest(raw_request);
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		parsing_error = e.what();
	}
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
		this->query_params = other.query_params;
		this->parsing_error = other.parsing_error;
		// Copy other necessary members
	}

	return *this;
}


void HttpRequest::parseRequest(const std::string &request)
{
	std::istringstream request_stream(request);
	std::string line;

	// Parse request line
	if (std::getline(request_stream, line))
	{
		if (line.empty() || line == "\r" || line == "\n")
			error("Empty request line encountered", "Request Parser");

		std::istringstream line_stream(line);
		parseRequestLine(line_stream);
		if (method == UNDEFINED || uri.empty() || version.empty())
			error("Malformed request line", "Request Parser");
	}

	// Parse headers
	while (std::getline(request_stream, line))
	{

		// Need to figure out how to handle the last empty newline with out triggering a runtime error
		if (line.empty() || line == "\r" || line == "\n")
		{
			// throw std::runtime_error("Empty header line encountered");
		}

		std::istringstream line_stream(line);
		parseHeaders(line_stream);
	}

	// Parse query parameters if present in URI
	if (uri.find('?') != std::string::npos)
	{
		size_t pos = uri.find('?');
		std::string query_string = uri.substr(pos + 1);
		uri = uri.substr(0, pos);
		parseQueryParams(query_string);
	}

	// Parse body
	while (std::getline(request_stream, line))
		body += line + "\n";
}

void HttpRequest::parseRequestLine(std::istringstream& line_stream)
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
		{
			error("Unsupported HTTP method: " + UNDEFINED, "Request Parser");
		}
	}
}

void HttpRequest::parseHeaders(std::istringstream& line_stream)
{
	std::string key, value;

	if (std::getline(line_stream, key, ':') && std::getline(line_stream, value))
	{
		key.erase(key.find_last_not_of(" \t\r\n") + 1);
		value.erase(0, value.find_first_not_of(" \t\r\n"));

		if (key.empty() || value.empty())
			error("Malformed header line: key or value is empty", "Request Parser");

		headers[key] = value;
	}
}

void HttpRequest::parseQueryParams(std::string const &query_string)
{
	std::istringstream query_stream(query_string);
	std::string pair;

	while (std::getline(query_stream, pair, '&'))
	{
		size_t pos = pair.find('=');

		if (pos != std::string::npos)
		{
			std::string key = pair.substr(0, pos);
			std::string value = pair.substr(pos + 1);
			query_params[key] = value;
		}
		else
			query_params[pair] = "";
	}
}

void HttpRequest::displayRequest() const
{
	std::cout << "-------HTTP REQUEST-------" << std::endl;
	std::cout << "Method: " << getMethodString(method)
	          << "\nURI: " << uri
	          << "\nVersion: " << version
	          << std::endl;
	std::cout << "-----------Headers-----------" << std::endl;

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		std::cout << it->first << ": " << it->second << std::endl;

	std::cout << "--------Query Params--------" << std::endl;

	for (std::map<std::string, std::string>::const_iterator it = query_params.begin(); it != query_params.end(); ++it)
		std::cout << it->first << ": " << it->second << std::endl;

	std::cout << "-----------Body-----------" << std::endl;
	std::cout << body << std::endl;
}
