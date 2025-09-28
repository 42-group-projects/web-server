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
	std::string serialize_headers(void);

	// Getters and Setters
	void set_version(const std::string &ver) { version = ver; };
	std::string get_version() const { return version; };

	void set_status(e_status_code stat) { status = stat; };
	e_status_code get_status() const { return status; };

	void set_mime_type(const std::string &type) { mime_type = type; };
	std::string get_mime_type() const { return mime_type; };

	void set_body(const std::string &b) { body = b; };
	std::string get_body() const { return body; }

	void set_header(const std::string &key, const std::string &value) { headers[key] = value; };
	std::string get_header(const std::string &key) const { return headers.at(key); };
	
	// For testing purposes
	std::string generateMockResponse(void);
	void mock_data(std::string const &body);
};

#endif
