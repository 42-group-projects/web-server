#include "HttpHandler.hpp"
#include "../utils.hpp"
#include "../CgiHandler/CgiHandler.hpp"

HttpHandler::HttpHandler() {}

HttpHandler::~HttpHandler() {}

// HttpResponse HttpHandler::handleRequest(const HttpRequest& req, const ServerConfig& config)
//==========================================================================================================
HttpResponse HttpHandler::handleRequest(const HttpRequest& req, const ServerConfig& config, std::string& ip, int port)
//===================================================================================================Clement
{

	// so i think i need to get the int from the net working layer to figure out witch config its going to be using here... need to talk to tsubo about it.
	// std::string server_name = config.getConfiguration()[0].server_name[0];
	// std::string ip = config.getAllListen()[0].first;
	// int port = config.getAllListen()[0].second;

	// std::cout << "Server Name: " << server_name << std::endl;
	// std::cout << "IP: " << ip << " Port: " << port << std::endl;
	// need to get this info from the network layer.
	// std::cout << req_config << std::endl;
	// FileSystem fs(req_config.safePath, req_config);
	// std::cout << fs  << std::endl;

	std::string server_name = req.getHeaders()["Host"];
	int pos = server_name.find_first_of(':');
	server_name = server_name.substr(0, pos);
	try
	{
		req_config = config.getRequestConfig(server_name, ip, port, req.getUri());
		// std::cout << req_config << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception caught in HttpHandler::handleRequest: 1";
		std::cerr << e.what() << '\n';
		return handleErrorPages(req, FORBIDDEN);
	}

	if(req.getBody().size() > req_config.client_max_body_size)
		return handleErrorPages(req, CONTENT_TOO_LARGE);

	// Reject requests that try to use both Content-Length and chunked Transfer-Encoding
	const std::map<std::string, std::string>& headers = req.getHeaders();
	std::map<std::string, std::string>::const_iterator teIt = headers.find("Transfer-Encoding");
	std::map<std::string, std::string>::const_iterator clIt = headers.find("Content-Length");

	if (teIt != headers.end() && clIt != headers.end())
	{
		std::string teVal = teIt->second;
		for (size_t i = 0; i < teVal.size(); ++i)
			teVal[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(teVal[i])));

		if (teVal.find("chunked") != std::string::npos)
			return handleErrorPages(req, BAD_REQUEST);
	}

	// CGI request
	try
	{
		CgiHandler cgi_handler(req_config);
		// if (false) // --- IGNORE ---
		if (isCgiRequest(req))
		{
			if (req_config.cgi_pass.empty())
				return handleErrorPages(req, FORBIDDEN);
			std::cout << "Handling CGI request for URI: " << req.getUri() << std::endl;
			return cgi_handler.runCgi(req);
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what();

		std::string msg(e.what());
		if (std::string(msg).find("File does not exist") != std::string::npos
			|| std::string(msg).find("code injection") != std::string::npos)
		{
			return handleErrorPages(req, NOT_FOUND);
		}
		else if (std::string(msg).find("permissions") != std::string::npos
			|| std::string(msg).find("unsafe") != std::string::npos)
		{
			return handleErrorPages(req, FORBIDDEN);
		}
		else if (std::string(msg).find("failed") != std::string::npos)
		{
			return handleErrorPages(req, INTERNAL_SERVER_ERROR);
		}
		else if(std::string(msg).find("terminated") != std::string::npos)
		{
			return handleErrorPages(req, INTERNAL_SERVER_ERROR);
		}
		//else fallback to normal handling
	}


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
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception caught in HttpHandler::handleRequest: 2\n";
		std::cerr << e.what() << '\n';
	}

	try
	{
		switch (req.getMethod())
		{
			case GET:
				return handleGet(req);

			case POST:
			 	return handlePost(req);

			case DELETE:
				return handleDelete(req);

			case PUT:
			case PATCH:
			case OPTIONS:
			case HEAD:
				return handleErrorPages(req, METHOD_NOT_ALLOWED);

			default:
				return handleErrorPages(req, NOT_IMPLEMENTED);
		}
	}
	catch(const std::exception& e)
	{

		std::cerr << "Exception caught in HttpHandler::handleRequest: 3\n";
		std::cerr << e.what() << '\n';
		return handleErrorPages(req, INTERNAL_SERVER_ERROR);
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

	(void)req;

	HttpResponse res;
	std::map<int, std::string> error_pages = req_config.error_pages;
	std::string error_page_path = error_pages[response_code];
	FileSystem fs(req_config.safePath, req_config);
	fs.errorPage(response_code, req_config);

	if(response_code == METHOD_NOT_ALLOWED)
		res.setHeader("Allow", addAllowHeaders());
	res.setStatus(response_code);
	res.setVersion("HTTP/1.1");
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	if(fs.exists())
		res.setBody(fs.getFileContents());
	return res;
}

HttpResponse HttpHandler::handleRedirects(const HttpRequest& req)
{

	(void)req;

	HttpResponse res;
	//Making headers
	res.setStatus(static_cast<e_status_code>(req_config.redirect_code));
	res.setVersion("HTTP/1.1");
	res.setHeader("Location", req_config.redirect_url);
	//Makiong Body
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
