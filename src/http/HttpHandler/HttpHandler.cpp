#include "HttpHandler.hpp"
#include "../utils.hpp"

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

	//checking for all parsing errors first
	if(!req.getParsingError().empty())
	{
		HttpResponse res;
		return res.badRequest(req);
	}
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
	req.displayRequest();

	FileSystem fs(SafePath(req.getUri()));
	if (fs.exists() && fs.readable() && !fs.directory())
	{
		// displayFileSystemInfo(fs);
		// std::cout << "File exists and is readable. Preparing response..." << std::endl;

		res.setStatus(OK);
		res.setVersion(req.getVersion());
		res.setMimeType(getMimeTypeString(fs.getMimeType()));
		res.setBody(fs.getFileContents());
	}
	return res;
}

