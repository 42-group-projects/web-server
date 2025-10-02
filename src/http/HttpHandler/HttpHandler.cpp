#include "HttpHandler.hpp"
#include "../utils.hpp"

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
	// TODO: will need to implemnt CGI handling here as well
	if(!req.getParsingError().empty())
	{
		return badRequest(req);
	}

	switch (req.getMethod())
	{
		case GET:
			return handleGet(req);

		case POST:
			// return handlePost(req);

		case DELETE:
			// return handleDelete(req);

		default:
			return methodNotAllowed(req);
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
	std::map<int, std::string> errorPages = config.getErrorPages();
	FileSystem fs(errorPages[METHOD_NOT_ALLOWED]);

	res.setStatus(METHOD_NOT_ALLOWED);
	res.setVersion(req.getVersion());
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	res.setBody(fs.getFileContents());
	return res;
}

HttpResponse HttpHandler::badRequest(const HttpRequest& req)
{
	HttpResponse res;
	std::map<int, std::string> errorPages = config.getErrorPages();
	FileSystem fs(errorPages[BAD_REQUEST]);

	res.setStatus(BAD_REQUEST);
	res.setVersion(req.getVersion());
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	res.setBody(fs.getFileContents());
	return res;
}


