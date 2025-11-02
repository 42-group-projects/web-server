#include "HttpHandler.hpp"
#include "../utils.hpp"
#include "../CgiHandler/CgiHandler.hpp"

HttpHandler::HttpHandler(const ServerConfig &config) : config(config) {}

HttpHandler::~HttpHandler() {}

HttpResponse HttpHandler::handleRequest(const HttpRequest& req)
{
	try
	{
		FileSystem fs = SafePath(req.getUri());
		LocationConfig location_config = config[fs];

		// std::cout << location_config << std::endl;
		// std::cout << fs << std::endl;

		CgiHandler cgi_handler(location_config);
		if (cgi_handler.isCgiRequest(req))
		{
			return cgi_handler.runCgi(req);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << "Falling back to standard request handling." << std::endl;
	}

	try
	{
		if(hasHttpRequestErrors(req))
		{
			std::cerr << "Bad Request: Empty URI or null byte detected." << std::endl;
			return handleErrorPages(req, BAD_REQUEST);
		}

		switch (req.getMethod())
		{
			case GET:
				return handleGet(req);

			case POST:
			 	return handlePost(req);

			case DELETE:
				return handleDelete(req);

			default:
				return handleErrorPages(req, BAD_REQUEST);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		//TODO: fiuger out what kind of response we want to send back here...
		return HttpResponse();
	}
}

bool HttpHandler::hasHttpRequestErrors(const HttpRequest& req)
{
	// req.displayRequest();
	// std::cout << g_config << std::endl;

	if((req.getVersion() != "HTTP/1.1") && (req.getVersion() != "HTTP/1.0"))
	{
		return true;
	}

	std::cout << "parsing error is: " << req.getParsingError() << std::endl;

	if(req.getParsingError() != "")
	{
		return true;
	}

	return false;
}

HttpResponse HttpHandler::handleErrorPages(const HttpRequest& req, e_status_code response_code)
{
	HttpResponse res;
	std::map<int, std::string> errorPages = config.getErrorPages();
	FileSystem fs(errorPages[response_code]);

	res.setStatus(response_code);
	res.setVersion(req.getVersion());
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	res.setBody(fs.getFileContents());
	return res;
}
