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

		uri = decodeUri(uri);

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
		if(!hasHost(headers))
		{
			error("Malformed header: Host name not present", "Request Parser");
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
		else if (method_str == "PUT")
			method = PUT;
		else if (method_str == "PATCH")
			method = PATCH;
		else if (method_str == "OPTIONS")
			method = OPTIONS;
		else if (method_str == "HEAD")
			method = HEAD;
		else
			method = UNDEFINED;

		if (uri.find("://") != std::string::npos)
		{
			parseAbsoluteUrl(uri);
		}
	}
}

void HttpRequest::parseAbsoluteUrl(std::string& absolute_url)
{
	size_t protocol_end = absolute_url.find("://");
	if (protocol_end == std::string::npos)
		return; // Not an absolute URL

	std::string protocol = absolute_url.substr(0, protocol_end);

	if (protocol != "http" && protocol != "https")
	{
		error("Unsupported protocol in absolute URL: " + protocol, "Request Parser");
		return;
	}

	std::string remainder = absolute_url.substr(protocol_end + 3);
	size_t path_start = remainder.find('/');

	std::string host_port;
	std::string path;

	if (path_start != std::string::npos)
	{
		host_port = remainder.substr(0, path_start);
		path = remainder.substr(path_start);
	}
	else
	{
		host_port = remainder;
		path = "/"; // Default path if none specified
	}

	// Extract host and port
	size_t port_start = host_port.find(':');
	if (port_start != std::string::npos)
	{
		this->host = host_port.substr(0, port_start);
		std::string port_str = host_port.substr(port_start + 1);

		std::istringstream iss(port_str);
		int port;
		if (!(iss >> port) || port <= 0 || port > 65535)
		{
			error("Invalid port number in absolute URL: " + port_str, "Request Parser");
			return;
		}
	}
	else
	{
		this->host = host_port;
	}
	// Validate host is not empty
	if (this->host.empty())
	{
		error("Empty host in absolute URL", "Request Parser");
		return;
	}
	setHeader("Host", this->host);
	size_t fragment_pos = path.find('#');
	if (fragment_pos != std::string::npos)
	{
		path = path.substr(0, fragment_pos);
	}
	this->uri = path;
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

		if (key == "Content-Length")
		{
			for (size_t i = 0; i < value.size(); ++i)
			{
				if (value[i] < '0' || value[i] > '9')
				{
					error("Invalid Content-Length header: " + value, "Request Parser");
				}
			}
		}

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
		warning("Content-Type header not found", "HttpRequest::getMimeTypeString");
	}
	return "text/plain";
}

std::string HttpRequest::decodeUri(const std::string& encoded_uri)
{
	std::string decoded;
	char ch;
	unsigned int i, j;

	for (i = 0; i < encoded_uri.length(); i++)
	{
		if (int(encoded_uri[i]) == 37 && i + 2 < encoded_uri.length()) // %
		{
			// Parse the hex digits after %
			if (sscanf(encoded_uri.substr(i + 1, 2).c_str(), "%x", &j) == 1)
			{
				ch = static_cast<char>(j);

				// Security check: Reject null bytes (potential null byte injection attack)
				if (ch == '\0')
				{
					error("Null byte detected in URI", "Request Parser");
					return encoded_uri; // Return original to avoid processing malformed URI
				}

				// Do NOT decode path separators and other reserved characters
				// %2F (/) should remain encoded to distinguish from literal /
				// %3F (?), %23 (#), etc. should also remain encoded
				if (ch == '/' || ch == '?' || ch == '#' || ch == '%')
				{
					// Keep the percent-encoded form for path-significant characters
					decoded += encoded_uri.substr(i, 3);
					i += 2;
				}
				else
				{
					// Decode other characters (spaces, etc.)
					decoded += ch;
					i += 2; // Skip the two hex digits
				}
			}
			else
			{
				// Invalid hex encoding, keep as-is
				decoded += encoded_uri[i];
			}
		}
		else
		{
			decoded += encoded_uri[i];
		}
	}
	return decoded;
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
