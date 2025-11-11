#include "HttpRequest.hpp"
#include "../utils.hpp"
#include "../../../include/imports.hpp"
// #include "../../errorHandling/ErrorWarning.hpp"

HttpRequest::HttpRequest() : method(UNDEFINED), uri(""), version(""), parsing_error(""){}

HttpRequest::~HttpRequest() {}

HttpRequest::HttpRequest(const std::string &raw_request) : method(UNDEFINED), uri(""), version(""), parsing_error("")
{
	try
	{
		parseRequest(raw_request);
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
	}
	return *this;
}

void HttpRequest::parseRequest(const std::string &request)
{
	std::istringstream request_stream(request);
	std::string line;

	try
	{
		// Parse request line
		std::getline(request_stream, line);
		if (line.empty() || line == "\r" || line == "\n")
			error("Empty request line encountered", "Request Parser");


		if(line.length() > MAX_REQUEST_LINE_LENGTH)
		{
			error("Max Request Line Limit breached", "Request Parser");
		}
		std::istringstream line_stream(line);
		parseRequestLine(line_stream);

		if (uri.empty() || version.empty() || getMethodString(method).empty())
		{
			error("Malformed request line", "Request Parser");
		}

		// Parse headers
		int header_count = 0;
		while (std::getline(request_stream, line))
		{
			line.erase(line.find_last_not_of(" \t\r\n") + 1);
			if (line.empty())
				break; // End of headers

			if(line.find(":") == std::string::npos)
				error("Malformed request line", "Request Parser");

			std::istringstream line_stream(line);
			parseHeaders(line_stream);
			header_count++;
			if (header_count > MAX_HEADERS_COUNT)
			{
				error("Exceeded maximum number of headers", "Request Parser");
			}
		}

		// query parameters
		if (uri.find('?') != std::string::npos)
		{
			size_t pos = uri.find('?');
			std::string query_string = uri.substr(pos + 1);
			uri = uri.substr(0, pos);
			setQuaryString(query_string);
			parseQueryParams(query_string);
		}

		// Parse body
		std::string body_line;
		while (std::getline(request_stream, body_line))
		{
			if (!body.empty())
				body += "\n";
			body += body_line;
		}
	}
	catch (const std::runtime_error& e)
	{
		std::cout << e.what() << std::endl;
		parsing_error = e.what();
	}

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
			method = UNDEFINED;
	}
}

void HttpRequest::parseHeaders(std::istringstream& line_stream)
{
	std::string key, value;

	if (std::getline(line_stream, key, ':') && std::getline(line_stream, value))
	{
		if(value[0] != ' ')
		{
			error("Malformed header line: Malformed", "Request Parser");
		}

		key.erase(key.find_last_not_of(" \t\r\n") + 1);
		value.erase(0, value.find_first_not_of(" \t\r\n"));

		if (key.empty() || value.empty() || key.size() > MAX_HEADERS_SIZE || value.size() > MAX_HEADERS_SIZE)
		{
			error("Malformed header line: key or value is empty", "Request Parser");
		}

		headers[key] = value;
	}
	// if(!hasHost(headers))
	// {
	// 	error("Malformed header: Host name not present", "Request Parser");
	// }
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

std::string HttpRequest::getMimeTypeString() const
{

	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Type");
	if (it != headers.end())
	{

		std::string trimmed = it->second;
		size_t end = trimmed.find_last_not_of(" \t\r\n");
		if (end != std::string::npos) {
			trimmed = trimmed.substr(0, end + 1);
		}

		size_t start = trimmed.find_first_not_of(" \t\r\n");
		if (start != std::string::npos) {
			trimmed = trimmed.substr(start);
		}

		return trimmed;
	}
	else
	{
		error("Content-Type header not found", "HttpRequest::getMimeTypeString");
	}
	return "text/plain";
}

void HttpRequest::displayRequest() const
{
	std::cout << "\n-------HTTP REQUEST-------" << std::endl;
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
