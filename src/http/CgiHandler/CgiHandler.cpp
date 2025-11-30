#include "CgiHandler.hpp"
#include "../utils.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cctype>
#include <algorithm>

CgiHandler::CgiHandler(const t_request_config& req_config)
	: req_config(req_config), cgi_interpreter(""), script_name(""), path_info("") {}

CgiHandler::~CgiHandler() {}

HttpResponse CgiHandler::runCgi(const HttpRequest& req)
{
	HttpRequest request = req;
	std::string output;
	int out_pipe[2];
	int in_pipe[2];
	int status_pipe[2];


	if (pipe(out_pipe) == -1 || pipe(status_pipe) == -1)
		throw std::runtime_error("pipe() failed in runCgi");

	bool is_post = (req.getMethod() == POST);
	if (is_post)
	{
		if (pipe(in_pipe) == -1)
			throw std::runtime_error("pipe() failed in runCgi");
	}

	pid_t pid = fork();
	if (pid == -1)
		throw std::runtime_error("fork() failed in runCgi");

	if (pid == 0)
	{
		detectCgiType(req);
		char **args = makeArgs(req);
		char **envp = makeEnvs(req);

		close(status_pipe[0]);
		char fail = 0;

		if (dup2(out_pipe[WRITE_FD], STDOUT_FILENO) == -1 ||
			dup2(out_pipe[WRITE_FD], STDERR_FILENO) == -1)
			fail = 1;

		close(out_pipe[READ_FD]);
		close(out_pipe[WRITE_FD]);

		if (is_post)
		{
			if (dup2(in_pipe[READ_FD], STDIN_FILENO) == -1)
				fail = 1;
			close(in_pipe[WRITE_FD]);
			close(in_pipe[READ_FD]);
		}

		if (fail == 0 && execve(cgi_interpreter.c_str(), args, envp) == -1)
		{
			fail = 1;
		}

		write(status_pipe[1], &fail, 1);
		close(status_pipe[1]);
		std::exit(0);
	}
	else
	{
		close(out_pipe[WRITE_FD]);
		close(status_pipe[1]);

		if (is_post)
		{
			close(in_pipe[READ_FD]);
			write(in_pipe[WRITE_FD], req.getBody().c_str(), req.getBody().size());
			close(in_pipe[WRITE_FD]);
		}

		char buf[4096];
		ssize_t n;
		while ((n = read(out_pipe[READ_FD], buf, sizeof(buf))) > 0)
		{
			output.append(buf, n);
		}
		close(out_pipe[READ_FD]);

		char child_status;
		read(status_pipe[0], &child_status, 1);
		close(status_pipe[0]);

		int wstatus;
		waitpid(pid, &wstatus, 0);

		if (child_status)
			throw std::runtime_error("CGI script failed to execute");

		if (!WIFEXITED(wstatus) || WEXITSTATUS(wstatus) != 0)
			throw std::runtime_error("CGI script terminated abnormally");
	}
	HttpResponse res;
	return res.parseCgiResponse(output);
}

void CgiHandler::detectCgiType(const HttpRequest& req)
{
	std::string uri = req.getUri();
	std::string extension = getExtension(uri);

	if (extension == ".cgi" || extension == ".sh")
	{
		cgi_interpreter = "/bin/sh";
	}
	else
	{
		std::map<std::string, std::string>::const_iterator it = req_config.cgi_pass.find(extension);
		if (it != req_config.cgi_pass.end())
		{
			cgi_interpreter = it->second;
		}
		else
		{
			cgi_interpreter = "";
		}
	}

	size_t ext_pos = uri.find(extension);
	if (ext_pos != std::string::npos)
		ext_pos += extension.size();

	script_name = uri.substr(0, ext_pos);
	if (ext_pos < uri.size())
		path_info = uri.substr(ext_pos);
	else
		path_info = "";
}



char **CgiHandler::makeArgs(const HttpRequest& req)
{
	detectCgiType(req);

	size_t last_slash = script_name.find_last_of('/');
	std::string script_base = (last_slash != std::string::npos) ? script_name.substr(last_slash + 1) : script_name;
	SafePath sp(script_name, req_config, false);
	if (sp.getFullPath()[0] != '/')
		sp.setFullPath("./" + sp.getFullPath());
	else
		sp.setFullPath("." + sp.getFullPath());
	
	FileSystem fs(sp, req_config);
	if (!fs.exists())
		error("File does not exist", "CgiHandler::makeArgs");
	if (!fs.executable())
		error("Missing permissions", "CgiHandler::makeArgs");

	char **args;
	if (cgi_interpreter.find("php-cgi") != std::string::npos) {
		args = new char*[2];
		args[0] = strdup(cgi_interpreter.c_str());
		args[1] = NULL;
	} else {
		args = new char*[3];
		args[0] = strdup(cgi_interpreter.c_str());
		args[1] = strdup(sp.getFullPath().c_str());
		args[2] = NULL;
	}
	return args;
}

char **CgiHandler::makeEnvs(const HttpRequest& req)
{
	if (cgi_interpreter.empty())
		detectCgiType(req);

	std::map<std::string, std::string> headers = req.getHeaders();
	std::map<std::string, std::string> env_vars;

	env_vars["PATH_INFO"] = path_info;
	env_vars["SCRIPT_NAME"] = script_name;
	env_vars["REQUEST_METHOD"] = getMethodString(req.getMethod());
	env_vars["SERVER_NAME"] = headers["Host"];
	env_vars["SERVER_PROTOCOL"] = req.getVersion();
	env_vars["QUERY_STRING"] = getQueryString(req);

	if (!req.getBody().empty())
	{
		env_vars["CONTENT_TYPE"] = req.getMimeTypeString();
		env_vars["CONTENT_LENGTH"] = headers["Content-Length"];
	}
	else
	{
		env_vars["CONTENT_TYPE"] = APPLICATION_OCTET_STREAM;
		env_vars["CONTENT_LENGTH"] = "0";
	}

	if (cgi_interpreter.find("php-cgi") != std::string::npos)
	{
		SafePath sp(script_name, req_config, false);
		env_vars["SCRIPT_FILENAME"] = sp.getFullPath();
		env_vars["REDIRECT_STATUS"] = "200";
		env_vars["SERVER_SOFTWARE"] = "webserv/1.0";
	}

	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::string key = it->first;
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		std::replace(key.begin(), key.end(), '-', '_');
		env_vars["HTTP_" + key] = it->second;
	}

	char **envp = new char*[env_vars.size() + 1];
	size_t i = 0;
	for (std::map<std::string, std::string>::iterator it = env_vars.begin(); it != env_vars.end(); ++it)
	{
		std::string env_entry = it->first + "=" + it->second;
		envp[i++] = strdup(env_entry.c_str());
	}
	envp[i] = NULL;

	return envp;
}

std::string CgiHandler::getQueryString(const HttpRequest& req)
{
	const std::string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-%=&";
	std::string qs = req.getQueryString();
	for (size_t i = 0; i < qs.size(); i++)
		if (allowed.find(qs[i]) == std::string::npos)
			error("Cannot run CGI, possible code injection", "CgiHandler::getQueryString");
	return qs;
}

std::string CgiHandler::getExtension(const std::string& uri)
{
	std::string ext;
	size_t pos = uri.find_last_of('.');
	if (pos == std::string::npos || pos == uri.size() - 1)
		return "";
	ext =  uri.substr(pos);
	pos = ext.find_first_of('/');
	if (pos == std::string::npos)
		return ext;
	return ext.substr(0, pos);
}
