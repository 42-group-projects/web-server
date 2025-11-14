#include "HttpHandler.hpp"
#include "../utils.hpp"
#include "../CgiHandler/CgiHandler.hpp"

HttpHandler::HttpHandler() {}

HttpHandler::~HttpHandler() {}

HttpResponse HttpHandler::handleRequest(const HttpRequest& req, const ServerConfig& config)
{
	// need to get IP Address and and make a get HOST fucntion to check against the server name. alos need the port number
	// need to get this info from the network layer.
	t_request_config request_config = config.getRequestConfig("localhost", "127.0.0.0", 8080, req.getUri());

	req_config = request_config;

	if(req.getBody().size() > MAX_PAYLOAD_SIZE)
		return handleErrorPages(req, CONTENT_TOO_LARGE);

	// CGI request
	try
	{
		FileSystem fs(req_config.safePath, req_config);
		// LocationConfig location_config = config[fs];

		CgiHandler cgi_handler(req_config);
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

	// Standard requestHttpHandler::HttpHandler(t_request_config req_config)
	try
	{
		if(hasHttpRequestErrors(req))
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
				return handleDelete(req);

			default:
				return handleErrorPages(req, NOT_IMPLEMENTED);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught in HttpHandler::handleRequest: ";
		std::cerr << e.what() << '\n';
		//TODO: fiuger out what kind of response we want to send back here...
		return handleErrorPages(req, INTERNAL_SERVER_ERROR);
		// return handleErrorPages(req, );
	}
}

bool HttpHandler::hasHttpRequestErrors(const HttpRequest& req)
{

	if((req.getVersion() != "HTTP/1.1") && (req.getVersion() != "HTTP/1.0"))
	{
		return true;
	}

	if(req.getParsingError() != "")
	{
		return true;
	}

	return false;
}

std::string  HttpHandler::addAllowHeaders( )
{
	std::stringstream allowed_methods;
	bool first = true;
	allowed_methods << " ";
	if (req_config.getAllowed || req_config.autoindex)
	{
		if (!first) allowed_methods << ", ";
		allowed_methods << "GET";
		first = false;
	}
	if (req_config.postAllowed || !req_config.upload_store.empty())
	{
		if (!first) allowed_methods << ", ";
		allowed_methods << "POST";
		first = false;
	}
	if (req_config.deleteAllowed)
	{
		if (!first) allowed_methods << ", ";
		allowed_methods << "DELETE";
		first = false;
	}

	return allowed_methods.str();
}

HttpResponse HttpHandler::handleErrorPages(const HttpRequest& req, e_status_code response_code)
{
	HttpResponse res;
	std::map<int, std::string> error_pages = req_config.error_pages;
	std::string error_page_path = error_pages[response_code];
	FileSystem fs(req_config.safePath, req_config);

	if(response_code == METHOD_NOT_ALLOWED)
		res.setHeader("Allow", addAllowHeaders());
	res.setStatus(response_code);
	res.setVersion(req.getVersion());
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	if(fs.exists())
		res.setBody(fs.getFileContents());
	return res;
}
