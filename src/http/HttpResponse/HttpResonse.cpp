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

std::string HttpResponse::generateResponse(e_status_code status)
{
	this->status = status;
	return serializeHeaders() + this->getBody();
}

std::string HttpResponse::serializeHeaders(void)
{
	std::ostringstream res;
	res << version << " " << status << " " <<  getStatusString(status) <<"\r\n";
	res << "Content-Type: " << mime_type << "\r\n";
	res << "Content-Length: " << body.size() << "\r\n";
	res << "Date: " << getCurrentTime() << "\r\n";
	res << "Server: " << g_config.getServerName() << "\r\n";

	// TODO: need to get the host ip address from the net work layer and set it here.
	// res << "Host: " << <HOST_IP_ADDRESS> << "\r\n";

	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if(!it->first.empty() && !it->second.empty())
		{
			res << it->first << ": " << it->second << "\r\n";
		}
		else
		{
			error("Malformed header: key or value is empty", "Response Serializer");
		}
	}

	res << "\r\n";
	return res.str();
}
