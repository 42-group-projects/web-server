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
				// return handleErrorPages(req, METHOD_NOT_ALLOWED);
			case DELETE:
			//  return handleDelete(req);
				return handleErrorPages(req, METHOD_NOT_ALLOWED);
			default:
				return handleErrorPages(req, METHOD_NOT_ALLOWED);
		}
	}
	catch(const std::exception& e)
	{	
		std::cerr << e.what() << '\n';
		// return handleErrorPages(req, INTERNAL_SERVER_ERROR);
		return HttpResponse();
	}
}

HttpResponse HttpHandler::handleGet(const HttpRequest& req)
{
	HttpResponse res;
	// req.displayRequest();
	res.setVersion(req.getVersion());

	FileSystem fs(SafePath(req.getUri()));
	if(!fs.exists())
		return handleErrorPages(req, NOT_FOUND);
	if(!fs.readable())
		return handleErrorPages(req, FORBIDDEN);
	if(fs.directory())
		return handleErrorPages(req, FORBIDDEN);
	
	res.setStatus(OK);
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	res.setBody(fs.getFileContents());
	return res;
}

HttpResponse HttpHandler::handlePost(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());

	FileSystem fs(SafePath(req.getUri()));
	LocationConfig location_config = config[fs];
	// displayLocationConfigDetails(location_config);
	// displayServerConfigDetails(config);
	// displayFileSystemInfo(fs);
	


	//post handling logic to be implemented

	return res;
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
