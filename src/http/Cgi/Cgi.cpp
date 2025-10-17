#include"Cgi.hpp"
#include "../utils.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>



CgiHandler::CgiHandler(const LocationConfig& config): location_config(config) {}
CgiHandler::~CgiHandler() {}

bool CgiHandler::isCgiRequest(const HttpRequest& req)
{
	FileSystem fs(SafePath(req.getUri()));
	SafePath safe_path(req.getUri());
	std::string uri = req.getUri();
	std::cout << fs << std::endl;

	if (location_config.cgi_pass.empty() || !fs.exists() || fs.directory() || !fs.readable() || !fs.executable())
	{
		std::cout << "CGI pass is empty or file does not exist or is directory or not readable/executable" << std::endl;
		return false;
	}

	if(uri.find(location_config.location) != 0)
	{
		std::cout << "Location prefix not found in URI" << std::endl;
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
	std::cout << "No matching CGI extension found" << std::endl;
	return false;
}

void printStringArray(char *arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%s\n", arr[i]);
    }
}
HttpRequest CgiHandler::runCgi(const HttpRequest& req)
{
	HttpRequest request = req;

	pid_t pid = fork();

	if(pid == 0)
	{ 
		std::cout << "_____________________inside cgi handler__________________________" << std::endl;
		std::string cgi_path = getCgiPath(req.getUri(), location_config);
		std::cout << "Locaiton config: " << location_config.root << "/test.sh" << std::endl;
		char cwd[PATH_MAX];
		std::cout << "PWD: " << getcwd(cwd, sizeof(cwd)) << std::endl;
		std::cout << "CGI Path: " << cgi_path.c_str() << std::endl;
		
		std::string string;
		string = location_config.root + "/test.sh";
		std::cout << "Script path: " << string << std::endl;
		
		char *path = strdup(string.c_str());
		

		// Set up environment variables for CGI
		char *args[3] = {strdup(cgi_path.c_str()), path, NULL };
		char **envp = makeEnv(req);
		// printStringArray(args, 1); // Print first 5 env variables for debugging
		printStringArray(envp, 5); // Print first 5 env variables for debugging
		
		if(execve(cgi_path.c_str(), args, envp) == -1)
		{
			error("Failed to execute CGI script", "CgiHandler::runCgi");
		}
	}
	waitpid(pid, NULL, 0);
	std::cout << "_____________________inside cgi handler__________________________" << std::endl;
	return request;
}

char **CgiHandler::makeEnv(const HttpRequest& req)
{
	std::map<std::string, std::string> headers = req.getHeaders();
	std::string uri = req.getUri();
	std::string script_name = uri;
	std::string query_string = req.getQueryString();
	std::cout << "Query string in makeEnv: " << query_string << std::endl;
	std::map<std::string, std::string> env_vars;

	// there are requered
	env_vars["REQUEST_METHOD"] = getMethodString(req.getMethod()).c_str();
	env_vars["SCRIPT_NAME"] = script_name.c_str();
	env_vars["SERVER_NAME"] = headers["Host"].c_str();
	env_vars["SERVER_PROTOCOL"] = req.getVersion().c_str();
	// env_vars["GATEWAY_INTERFACE", req.getVersion().c_str();


	// these are only  if there are in the request
	if (!query_string.empty())
		env_vars["QUERY_STRING"] = query_string.c_str();
	else
		env_vars["QUERY_STRING"] = "";

	if(req.getBody().empty() == false)
	{
		env_vars["CONTENT_TYPE"] = req.getMimeTypeString().c_str();
		env_vars["CONTENT_LENGTH"] = headers["Content-Length"].c_str();
	}
	// set other ENV variables from headers.....

	char **envp = NULL;
	envp = new char*[env_vars.size() + 1];
	int i = 0;
	for(std::map<std::string, std::string>::iterator it = env_vars.begin(); it != env_vars.end(); ++it)
	{
		std::string env_entry = it->first + "=" + it->second;

		envp[i] = strdup(env_entry.c_str());
		i++;
	}
	envp[i] = NULL; // Null-terminate the array
	return envp;
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

