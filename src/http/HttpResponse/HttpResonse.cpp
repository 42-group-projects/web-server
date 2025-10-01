#include "./HttpResponse.hpp"
#include "../utils.hpp"

HttpResponse::HttpResponse() : version("Http1.1"), status(UNSET), mime_type(""), body("") {}

HttpResponse::~HttpResponse() {}

HttpResponse::HttpResponse(HttpResponse const &other)
{
	if (this != &other)
	{
		this->version = other.version;
		this->status = other.status;
		this->mime_type = other.mime_type;
		this->headers = other.headers;
		this->body = other.body;
	}
}
HttpResponse &HttpResponse::operator=(HttpResponse const &other)
{
	if (this != &other)
	{
		this->version = other.version;
		this->status = other.status;
		this->mime_type = other.mime_type;
		this->headers = other.headers;
		this->body = other.body;
	}

	return *this;
}

std::string HttpResponse::generateResponse(e_status_code status, const std::string &body)
{
	this->status = status;
	this->body = body;
	return serializeHeaders() + body;
}

std::string HttpResponse::serializeHeaders(void)
{
	std::ostringstream res;
	res << version << " " << status << "\r\n";
	res << "Content-Type: " << mime_type << "\r\n";
	res << "Content-Length: " << body.size() << "\r\n";
	res << "Date: " << get_current_time() << "\r\n";

	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
		res << it->first << ": " << it->second << "\r\n";

	res << "\r\n";
	return res.str();
}

std::string HttpResponse::generateMockResponse(void)
{
	std::cout << "Generating mock HTTP response..." << std::endl;
	version = "HTTP/1.1";
	status = OK;
	mime_type = "text/html";
	// Adding mock header data
	headers["Date"] = get_current_time();
	headers["Server"] = "WebServer/1.0";
	headers["Connection"] = "keep-alive";
	headers["Cache-Control"] = "no-cache";
	headers["Access-Control-Allow-Origin"] = "*";
	body = "<html><body><h1>Hello, World!</h1></body></html>";
	return serializeHeaders() + body;
}

void HttpResponse::mockData(std::string const &data)
{
	if (!data.empty())
	{
		version = "HTTP/1.1";
		status = OK;
		mime_type = "text/html";
		headers["Content-Type"] = "text/html";
		headers["Date"] = get_current_time();
		headers["Server"] = "WebServer/1.0";
		headers["Connection"] = "close";
		body = "<html><body><h1>Hello, World!</h1></body></html>";
	}
	else
	{
		version = "HTTP/1.1";
		status = OK;
		mime_type = "text/html";
		headers["Content-Type"] = "text/html";
		headers["Date"] = get_current_time();
		headers["Server"] = "WebServer/1.0";
		headers["Connection"] = "close";
		body = data;
	}

	std::cout << "Mock data function called." << std::endl;
}
