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
	// TODO: need to get the host name from the net work layer and set it here.
	res << "Server: " << "placeholder" << "\r\n";
	res << "Host: "  << "place holder "<<"\r\n";

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

HttpResponse &HttpResponse::parseCgiResponse(const std::string& cgi_output)
{
	std::cout << "Parsing CGI response..." << cgi_output << std::endl;
	int cut_size = 0;
	size_t header_end = cgi_output.find("\r\n\r\n");
	cut_size = 4;
	if(header_end == std::string::npos)
	{
		header_end = cgi_output.find("\n\n");
		cut_size = 2;
	}
	if(header_end == std::string::npos)
	{
		header_end = cgi_output.find("\r\r");
		cut_size = 2;
	}
	if (header_end == std::string::npos)
	{
		this->setStatus(INTERNAL_SERVER_ERROR);
		this->setBody("Malformed CGI response: Missing header-body separator.");
		return *this;
	}

	std::string header_section = cgi_output.substr(0, header_end);
	std::string body_section = cgi_output.substr(header_end + cut_size);

	std::istringstream header_stream(header_section);
	std::string line;
	bool status_set = false;

	while (std::getline(header_stream, line))
	{
		size_t colon_pos = line.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);
			value.erase(0, value.find_first_not_of(" "));

			if (key == "Status")
			{
				int status_code = atoi(value.c_str());
				this->setStatus(static_cast<e_status_code>(status_code));
				status_set = true;
			}
			else if(key == "Content-Type")
			{
				this->setMimeType(value);
			}
			else
			{
				this->setHeader(key, value);
			}
		}
	}

	if (!status_set)
	{
		this->setStatus(OK);
	}

	this->setBody(body_section);
	return *this;
}
