#include"CgiHandler.hpp"
#include "../utils.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>


CgiHandler::CgiHandler(const t_request_config& req_config): req_config(req_config) {}
CgiHandler::~CgiHandler() {}

HttpResponse CgiHandler::runCgi(const HttpRequest& req)
{
	HttpRequest request = req;
	std::string output;
	int out_pipe[2];
	int in_pipe[2]; // for POST body

	if (pipe(out_pipe) == -1)
		error("pipe() failed", "CgiHandler::runCgi");

	bool is_post = (req.getMethod() == POST);
	if (is_post && pipe(in_pipe) == -1)
		error("pipe() failed", "CgiHandler::runCgi");

	pid_t pid = fork();
	if (pid == -1)
		error("fork() failed", "CgiHandler::runCgi");

	if (pid == 0)
	{
		std::string cgi_path = getCgiPath(req.getUri(), req_config);
		char **args = makeArgs(req);
		char **envp = makeEnvs(req);

		// stdout and stderr
		if (dup2(out_pipe[WRITE_FD], STDOUT_FILENO) == -1 || dup2(out_pipe[WRITE_FD], STDERR_FILENO) == -1)
			error("dup2() failed", "CgiHandler::runCgi");
		close(out_pipe[READ_FD]);
		close(out_pipe[WRITE_FD]);

		if (is_post)
		{
			if (dup2(in_pipe[READ_FD], STDIN_FILENO) == -1)
				error("dup2() failed for stdin", "CgiHandler::runCgi");
			close(in_pipe[WRITE_FD]);
			close(in_pipe[READ_FD]);
		}

		if (execve(cgi_path.c_str(), args, envp) == -1)
			error("Failed to execute CGI script", "CgiHandler::runCgi");

		exit(EXIT_FAILURE);
	}
	else
	{
		// parent process
		close(out_pipe[WRITE_FD]);
		if (is_post)
		{
			close(in_pipe[READ_FD]);
			write(in_pipe[WRITE_FD], req.getBody().c_str(), req.getBody().size());
			close(in_pipe[WRITE_FD]);
		}

		char buf[4096];
		ssize_t n;
		while ((n = read(out_pipe[READ_FD], buf, sizeof(buf))) > 0)
			output.append(buf, n);
		close(out_pipe[READ_FD]);

		waitpid(pid, NULL, 0);
	}

	HttpResponse res;
	return res.parseCgiResponse(output);
}


char **CgiHandler::makeEnvs(const HttpRequest& req)
{
	std::map<std::string, std::string> headers = req.getHeaders();
	std::string script_name = req.getUri();
	std::string query_string = req.getQueryString();
	std::map<std::string, std::string> env_vars;

	// there are requered
	env_vars["REQUEST_METHOD"] = getMethodString(req.getMethod()).c_str();
	env_vars["SCRIPT_NAME"] = script_name.c_str();
	//need to plit this HOST up into SERVER_NAME and SERVER_PORT
	env_vars["SERVER_NAME"] = headers["Host"].c_str();
	// env_vars["SERVER_PORT"] = req_config.port.c_str();
	env_vars["SERVER_PROTOCOL"] = req.getVersion().c_str();
	// env_vars["GATEWAY_INTERFACE", req.getVersion().c_str();

	// these are only if there are in the request
	if (!query_string.empty())
		env_vars["QUERY_STRING"] = query_string.c_str();
	else
		env_vars["QUERY_STRING"] = "";

	if(req.getBody().empty() == false)
	{
		env_vars["CONTENT_TYPE"] = req.getMimeTypeString().c_str();
		env_vars["CONTENT_LENGTH"] = headers["Content-Length"].c_str();
	}

	for(std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::string key = it->first;;
		std::string value = it->second;
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		std::replace(key.begin(), key.end(), '-', '_');
		std::string env_key = "HTTP_" + key;
		env_vars[env_key] = value.c_str();
	}

	char **envp = new char*[env_vars.size() + 1];
	int i = 0;
	for(std::map<std::string, std::string>::iterator it = env_vars.begin(); it != env_vars.end(); ++it)
	{
		std::string env_entry = it->first + "=" + it->second;

		envp[i] = strdup(env_entry.c_str());
		i++;
	}
	envp[i] = NULL;
	return envp;
}

char **CgiHandler::makeArgs(const HttpRequest& req)
{
	char **args = new char*[3];

	std::string cgi_path = getCgiPath(req.getUri(), req_config);

	std::string script_filename = req.getUri();


	// TODO: NEED TO FIX THIS PATH FINDING LOGIC
	size_t last_slash = script_filename.find_last_of('/');

	std::string script_path;
	if (last_slash != std::string::npos)
	{
		script_path = script_filename.substr(last_slash + 1);
	}
	std::cout <<"URI is: " << req.getUri() << std::endl;
	std::cout <<"Script filename is: " << script_filename << std::endl;
	std::cout <<"CGI Path is: " << cgi_path << std::endl;
	std::cout <<"Root is: " << req_config.root << std::endl;

	if(req_config.root[0] == '/')
	{
		script_path =  "." + req_config.root + "/" + script_path;
	}
	else
	{
		script_path =  "./" + req_config.root + "/" + script_path;
	}

	// script_path = "." + req.getUri();

	args[0] = strdup(cgi_path.c_str());
	args[1] = strdup(script_path.c_str());
	args[2] = NULL;
	return args;
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

std::string CgiHandler::getCgiPath(std::string uri, const t_request_config& req_config)
{
	std::string cgi_path;
	std::string extension = getExtention(uri);

	// std::cout << "CGI Extension: " << extension << std::endl;
	if(extension == ".cgi")
	{
		// this need to brought in from the fisrt line of all .cgi files
		return "/bin/sh"; // default cgi interpreter for .cgi files
	}
	if(extension == ".sh")
	{
		return "/bin/sh";
	}

	std::map<std::string, std::string>::const_iterator it = req_config.cgi_pass.find(extension);
	if (it != req_config.cgi_pass.end())
	{
		std::cout << "CGI Interpreter for extension " << extension << ": " << it->second << std::endl;
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
