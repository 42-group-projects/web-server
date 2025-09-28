#include "./HttpResponse.hpp"

HttpResponse::HttpResponse() : status(UNSET), body("")
{
	std::cout << "HttpResponse constructor called" << std::endl;
}
HttpResponse::~HttpResponse()
{
	std::cout << "HttpResponse destructor called" << std::endl;
}
void HttpResponse::generateResponse(const std::string &status, const std::string &body)
{
	// This is just to show that the function is called
	(void)status;
	(void)body;
	std::cout << "Status: " << this->status << "\nBody: " << body << std::endl;
}
