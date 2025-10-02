#include "HttpHandler.hpp"
#include "../utils.hpp"

// HttpHandler::HttpHandler() : config()
// {
// 	std::cout << "HttpHandler constructor called" << std::endl;
// }

HttpHandler::HttpHandler(const ServerConfig &config) : config(config)
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
			return methodNotAllowed(req);
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
		res.setStatus(OK);
		res.setVersion(req.getVersion());
		res.setMimeType(getMimeTypeString(fs.getMimeType()));
		res.setBody(fs.getFileContents());
	}
	return res;
}

HttpResponse HttpHandler::methodNotAllowed(const HttpRequest& req)
{
	HttpResponse res;
	// config.getErrorPages();
	std::map<int, std::string> errorPages = config.getErrorPages();
	for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}

	res.setStatus(METHOD_NOT_ALLOWED);
	res.setVersion(req.getVersion());
	res.setMimeType("text/html"); // need to figure this out later
	res.setBody(errorPages[METHOD_NOT_ALLOWED]); // need to figure this out later

	res.setHeader("Connection", "close");
	return res;
}
