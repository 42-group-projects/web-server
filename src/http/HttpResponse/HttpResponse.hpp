#include "../include/imports.hpp"
#include "../include/enums.hpp"

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

class HttpResponse
{
public:
	HttpResponse();
	~HttpResponse();
	void generateResponse(const std::string &status, const std::string &body);
private:
	e_status_code status;
	std::string headers;
	std::string mime_type;
	std::string body;
};

#endif
