#include "HttpHandler.hpp"
#include "../utils.hpp"

HttpHandler::HttpHandler(const ServerConfig &config) : config(config) {}

HttpHandler::~HttpHandler() {}

HttpResponse HttpHandler::handleRequest(const HttpRequest& req)
{
	// TODO: will need to implemnt CGI handling here as well
	try
	{
		if(!req.getParsingError().empty())
		{
			return handleErrorPages(req, BAD_REQUEST);
		}
		
		switch (req.getMethod())
		{
			case GET:
				return handleGet(req);

			case POST:
			 	return handlePost(req);

			case DELETE:
				return handleErrorPages(req, NOT_IMPLEMENTED);
			//  return handleDelete(req);

			default:
				return handleErrorPages(req, METHOD_NOT_ALLOWED);
		}
	}
	catch(const std::exception& e)
	{	
		std::cerr << e.what() << '\n';
		//TODO: fiuger out what kind of response we want to send back here...
		return HttpResponse();
	}
}

HttpResponse HttpHandler::handleErrorPages(const HttpRequest& req, e_status_code response_code)
{
	HttpResponse res;
	std::map<int, std::string> errorPages = config.getErrorPages();
	FileSystem fs(errorPages[response_code]);

	res.setStatus(response_code);
	res.setVersion(req.getVersion());
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	// add more headers if needed
	res.setBody(fs.getFileContents());
	return res;
}
