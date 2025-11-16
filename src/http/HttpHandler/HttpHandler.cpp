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

	std::cout << request_config << std::endl;
	
	req_config = request_config;

	if(req.getBody().size() > MAX_PAYLOAD_SIZE)
		return handleErrorPages(req, CONTENT_TOO_LARGE);

	// CGI request
	try
	{
		// FileSystem fs(req_config.safePath, req_config);
		// LocationConfig location_config = config[fs];

		CgiHandler cgi_handler(req_config);
		// if (isCgiRequest(req))
		if (false) // --- IGNORE ---
		{
			std::cout << "Handling CGI request for URI: " << req.getUri() << std::endl;
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

		if (req_config.redirect_code != 0 && !req_config.redirect_url.empty())
		{
			return handleRedirects(req);
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

std::string  HttpHandler::addAllowHeaders()
{
	std::stringstream allowed_methods;

	std::cout << "getAllowed : " << req_config.getAllowed << std::endl;
	std::cout << "postAllowed : " << req_config.postAllowed << std::endl;
	std::cout << "deleteAllowed : " << req_config.deleteAllowed << std::endl;

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

HttpResponse HttpHandler::handleRedirects(const HttpRequest& req)
{
	HttpResponse res;
	res.setStatus(static_cast<e_status_code>(req_config.redirect_code)); // e.g. 301 or 302
	res.setVersion(req.getVersion());
	res.setHeader("Location", req_config.redirect_url);
	// Minimal body for HTTP/1.0 clients
	std::stringstream string_stream;
	string_stream  << "<html><head><title>Redirect</title></head><body>"
					<< "<h1>Resource moved</h1><p><a href=\"" << req_config.redirect_url
					<< "\">" << req_config.redirect_url << "</a></p></body></html>";
	res.setMimeType("text/html");
	res.setBody(string_stream.str());

	return res;
}

bool HttpHandler::isCgiRequest(const HttpRequest& req)
{
	// Determine if requested URI targets a CGI script.
	// Criteria:
	// - The URI starts with the configured location prefix (e.g., /cgi-bin)
	// - The script name (before any PATH_INFO and query) ends with ".cgi" OR
	//   has an extension present in req_config.cgi_pass keys (e.g., .py, .php, .sh)
	std::string uri = req.getUri();

	// Strip query string
	size_t qpos = uri.find('?');
	if (qpos != std::string::npos)
		uri = uri.substr(0, qpos);

	const std::string& loc = req_config.location;
	if (loc.empty())
		return false;
	if (uri.compare(0, loc.size(), loc) != 0)
		return false;

	// Scan to find the extension of the script BEFORE PATH_INFO
	size_t afterLoc = loc.size();
	bool seenDot = false;
	size_t dotPos = std::string::npos;
	size_t i = afterLoc;
	for (; i < uri.size(); ++i)
	{
		char c = uri[i];
		if (c == '/')
		{
			if (seenDot)
				break; // PATH_INFO begins after the script name
			else
				continue; // still traversing directories before script
		}
		if (c == '.' && !seenDot)
		{
			dotPos = i;
			seenDot = true;
		}
	}

	std::string ext;
	if (seenDot && dotPos != std::string::npos)
		ext = uri.substr(dotPos, i - dotPos);

	if (ext == ".cgi")
		return true;

	if (!ext.empty())
	{
		if (req_config.cgi_pass.find(ext) != req_config.cgi_pass.end())
			return true;
	}

	return false;
}

