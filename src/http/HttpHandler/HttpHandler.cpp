#include "HttpHandler.hpp"

HttpHandler::HttpHandler()
{
	std::cout << "HttpHandler constructor called" << std::endl;
}

HttpHandler::~HttpHandler()
{
	std::cout << "HttpHandler destructor called" << std::endl;
}

HttpResponse HttpHandler::handleRequest(const HttpRequest& req)
{
	// will need to implemnt CGI handling here as well
	switch (req.getMethod())
	{
		case GET:
			return handleGet(req);
			break;
		case POST:
			// return handlePost(req);
			break;
		case DELETE:
			// return handleDelete(req);
			break;
		default:
			// Need to implement a default response for unsupported methods
			break;
	}
	return HttpResponse();
}

HttpResponse HttpHandler::handleGet(const HttpRequest& req)
{
	HttpResponse res;

	(void)req; // to avoid unused parameter warning for now

	FileSystem fs(SafePath("/html/index.html"));

	// if (!fs.exists())
	// {
		displayFileSystemInfo(fs);
	// }
	// Implement GET handling logic here
	return res;
}

