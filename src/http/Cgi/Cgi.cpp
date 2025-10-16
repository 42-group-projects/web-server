#include"Cgi.hpp"
#include "../utils.hpp"
#include <sys/stat.h>
#include <unistd.h>

CgiHandler::CgiHandler(const LocationConfig& config): location_config(config) {}
CgiHandler::~CgiHandler() {}

bool CgiHandler::isCgiRequest(const HttpRequest& req)
{
	FileSystem fs(SafePath(req.getUri()));
	std::string uri = req.getUri();
	if (location_config.cgi_pass.empty() || !fs.exists() || fs.directory() || !fs.readable() || !fs.executable())
	{
		return false;
	}

	if(uri.find(location_config.location) != 0)
	{
		return false;
	}
	std::string extension = getExtention(uri);
	for (std::map<std::string, std::string>::const_iterator it = location_config.cgi_pass.begin(); it != location_config.cgi_pass.end(); ++it)
	{
		if(it->first == extension)
		{
			return true;
		}
	}

	return false;
}
HttpRequest CgiHandler::runCgi(const HttpRequest& req)
{
	HttpRequest request = req;

	pid_t pid = fork();

	if(pid == 0)
	{
		std::string cgi_path = getCgiPath(req.getUri(), location_config);
		std::cout << "CGI Path: " << cgi_path << std::endl;
		char cwd[PATH_MAX];
   		std::cout << "PWD: " << getcwd(cwd, sizeof(cwd)) << std::endl;
		setEnvs(req, location_config);
	}
	return request;
}

void CgiHandler::setEnvs(const HttpRequest& req, const LocationConfig& config)
{
	(void)config;
	std::map<std::string, std::string> headers = req.getHeaders();
	std::string uri = req.getUri();
	std::string script_name = uri;
	std::string query_string = getQuaryString(uri);

	// there are requered
	setenv("REQUEST_METHOD", getMethodString(req.getMethod()).c_str(), 1);
	setenv("SCRIPT_NAME", script_name.c_str(), 1);
	setenv("SEVER_NAME", headers["Host"].c_str(), 1);
	setenv("SERVER_PROTOCOL", req.getVersion().c_str(), 1);
	// setenv("GATEWAY_INTERFACE", req.getVersion().c_str(), 1);


	// these are only set if there are in the request
	if (!query_string.empty())
		setenv("QUERY_STRING", query_string.c_str(), 1);
	else
		setenv("QUERY_STRING", "", 1);

	if(req.getBody().empty() == false)
	{
		setenv("CONTENT_TYPE", req.getMimeTypeString().c_str(), 1);
		setenv("CONTENT_LENGTH", headers["Content-Length"].c_str(), 1);
	}



	//if (!req.pathInfo.empty())
	// {
    //     setenv("PATH_INFO", req.pathInfo.c_str(), 1);
    //     setenv("PATH_TRANSLATED", req.pathTranslated.c_str(), 1);
    // }

    // // Authentication (if HTTP authentication is used)
    // string authType = req.getAuthType();  // "Basic" or "Digest"
    // if (!authType.empty())
	// {
    //     setenv("AUTH_TYPE", authType.c_str(), 1);
    //     setenv("REMOTE_USER", req.getAuthUser().c_str(), 1);
    // }
}

std::string CgiHandler::getQuaryString(const std::string& uri)
{
	std::string query_string;
	size_t query_pos = uri.find('?');
	if (query_pos != std::string::npos)
	{
		query_string = uri.substr(query_pos + 1);
	}
	return query_string;
}

std::string CgiHandler::getCgiPath(std::string uri, const LocationConfig& config)
{
	std::string cgi_path;
	std::string extension = getExtention(uri);
	std::map<std::string, std::string>::const_iterator it = config.cgi_pass.find(extension);
	if (it != config.cgi_pass.end())
	{
		cgi_path = it->second;
	}
	return cgi_path;
}

std::string CgiHandler::getExtention(const std::string& uri)
{
	size_t dot_pos = uri.find_last_of('.');
	if (dot_pos == std::string::npos || dot_pos == uri.length() - 1) {
		return "";
	}
	return uri.substr(dot_pos);
}