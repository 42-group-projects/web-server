#include "HttpHandler.hpp"
#include "../utils.hpp"
#include "../CgiHandler/CgiHandler.hpp"

HttpHandler::HttpHandler() {}

HttpHandler::~HttpHandler() {}

bool HttpHandler::loadRequestConfig(const HttpRequest& req, const ServerConfig& config, const std::string& ip, int port)
{
    std::string server_name = req.getHeaders().count("Host") ? req.getHeaders().at("Host") : "";
    size_t pos = server_name.find(':');
    if (pos != std::string::npos)
        server_name = server_name.substr(0, pos);

    try {
        req_config = config.getRequestConfig(server_name, ip, port, req.getUri());
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[HttpHandler] Failed to get request config: " << e.what() << std::endl;
        return false;
    }
}

HttpResponse HttpHandler::handleRequest(const HttpRequest& req, const ServerConfig& config, std::string& ip, int port)
{
	if (!loadRequestConfig(req, config, ip, port))
	{
		return handleErrorPages(req, FORBIDDEN);
	}

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
    std::cout << "\n[DEBUG] Entered isCgiRequest()\n";

    std::string uri = req.getUri();
    std::cout << "[DEBUG] Original URI: '" << uri << "'\n";

    // Strip query string
    size_t qpos = uri.find('?');
    if (qpos != std::string::npos) {
        uri = uri.substr(0, qpos);
        std::cout << "[DEBUG] URI after stripping query string: '" << uri << "'\n";
    }

    const std::string& loc = req_config.location;
    std::cout << "[DEBUG] CGI location configured: '" << loc << "'\n";

    if (loc.empty()) {
        std::cout << "[DEBUG] Location is empty -> not CGI\n";
        return false;
    }

    if (uri.compare(0, loc.size(), loc) != 0) {
        std::cout << "[DEBUG] URI does not match location prefix -> not CGI\n";
        return false;
    }

    std::cout << "[DEBUG] URI matches location prefix\n";

    size_t afterLoc = loc.size();
    bool seenDot = false;
    size_t dotPos = std::string::npos;
    size_t i = afterLoc;

    for (; i < uri.size(); ++i)
    {
        char c = uri[i];
        if (c == '/')
        {
            if (seenDot) {
                std::cout << "[DEBUG] Found '/' after dot at i=" << i << " -> end of script name\n";
                break; // PATH_INFO begins after the script name
            } else {
                continue; // still traversing directories before script
            }
        }
        if (c == '.' && !seenDot)
        {
            dotPos = i;
            seenDot = true;
            std::cout << "[DEBUG] Found '.' at position " << dotPos << "\n";
        }
    }

    std::string ext;
    if (seenDot && dotPos != std::string::npos)
        ext = uri.substr(dotPos, i - dotPos);

    std::cout << "[DEBUG] Extracted extension: '" << ext << "'\n";

    if (ext == ".cgi") {
        std::cout << "[DEBUG] Extension is .cgi -> CGI request!\n";
        return true;
    }

    if (!ext.empty()) {
        if (req_config.cgi_pass.find(ext) != req_config.cgi_pass.end()) {
            std::cout << "[DEBUG] Extension found in cgi_pass -> CGI request!\n";
            return true;
        } else {
            std::cout << "[DEBUG] Extension not in cgi_pass -> not CGI\n";
        }
    } else {
        std::cout << "[DEBUG] No extension found -> not CGI\n";
    }

    std::cout << "[DEBUG] Exiting isCgiRequest() -> not CGI\n";
    return false;
}
