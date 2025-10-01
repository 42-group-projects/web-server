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

	if(!req.getParsingError().empty())
	{
		HttpResponse res;
		return res.badRequest(req);
	}

	// also parsing handling error here as well
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
	req.displayRequest();
	FileSystem fs(SafePath(req.getUri()));
	std::cout << "after fs creation" << std::endl;
	if (fs.exists())
	{
		displayFileSystemInfo(fs);
	}
	// Implement GET handling logic here
	return res;
}

