#include "../include/imports.hpp"
#include "../include/enums.hpp"

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

class HttpResponse
{
private:
	std::string version;
	e_status_code status;
	std::string mime_type;
	std::map<std::string , std::string> headers;
	std::string body;
public:
	HttpResponse();
	HttpResponse(HttpResponse const &other);
	HttpResponse &operator=(HttpResponse const &other);
	~HttpResponse();


	std::string generateResponse(e_status_code status, const std::string &body);
	std::string serializeHeaders(void);

	// Getters and Setters
	void setVersion(const std::string &ver) { version = ver; };
	std::string getVersion() const { return version; };

	void setStatus(e_status_code stat) { status = stat; };
	e_status_code getStatus() const { return status; };

	void setMimeType(const std::string &type) { mime_type = type; };
	std::string getMimeType() const { return mime_type; };

	void setBody(const std::string &b) { body = b; };
	std::string getBody() const { return body; }

	void setHeader(const std::string &key, const std::string &value) { headers[key] = value; };
	std::string getHeader(const std::string &key) const { return headers.at(key); };
	
	// For testing purposes
	std::string generateMockResponse(void);
	void mockData(std::string const &body);
};

#endif
