#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "../HttpRequest/HttpRequest.hpp"

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

class HttpResponse
{
private:
	std::string version;
	e_status_code status;
	std::string mime_type;
	std::map<std::string, std::string> headers;
	std::string body;

public:
	// Constructors, destructor, and assignment operator
	HttpResponse();
	HttpResponse(HttpResponse const &other);
	HttpResponse &operator=(HttpResponse const &other);
	~HttpResponse();

	// Main functions
	std::string generateResponse(e_status_code status);
	std::string serializeHeaders(void);

	//Status code
	HttpResponse badRequest(HttpRequest const &req);
	// HttpResponse notFound(void);
	// HttpResponse internalServerSrror(void);
	// HttpResponse methodNotAllowed(void);
	// HttpResponse notImplemented(void);
	// HttpResponse forbidden(void);

	// Getters
	std::string getVersion() const { return version; };
	e_status_code getStatus() const { return status; };
	std::string getMimeType() const { return mime_type; };
	std::string getBody() const { return body; }
	std::string getHeader(const std::string &key) const { return headers.at(key); };

	// Setters
	void setVersion(const std::string &ver) { version = ver; };
	void setStatus(e_status_code stat) { status = stat; };
	void setMimeType(const std::string &type) { mime_type = type; };
	void setBody(const std::string &b) { body = b; };
	void setHeader(const std::string &key, const std::string &value) { headers[key] = value; };

	// For testing purposes
	std::string generateMockResponse(void);
	void mockData(std::string const &body);
};

#endif
