#include"CgiHandler.hpp"
#include "../utils.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>


CgiHandler::CgiHandler(const t_request_config& req_config): req_config(req_config) {}
CgiHandler::~CgiHandler() {}

bool CgiHandler::isCgiRequest(const HttpRequest& req)
{

	FileSystem fs(req_config.safePath, req_config);

	std::string uri = req.getUri();

	if (req_config.cgi_pass.empty() || !fs.exists() || fs.directory() || !fs.readable() || !fs.executable())
	{
		return false;
	}

	if(uri.find(req_config.location) != 0)
	{
		std::cout << "Location prefix not found in URI" << std::endl;
		return false;
	}

	std::string extension = getExtention(uri);
	for (std::map<std::string, std::string>::const_iterator it = req_config.cgi_pass.begin(); it != req_config.cgi_pass.end(); ++it)
	{
		if(it->first == extension)
		{
			return true;
		}
	}
	std::cout << "No matching CGI extension found" << std::endl;
	return false;
}

HttpResponse CgiHandler::runCgi(const HttpRequest& req)
{
	HttpRequest request = req;
	std::string output;
	int pipefd[2];

	if (pipe(pipefd) == -1)
	{
		error("pipe() failed", "CgiHandler::runCgi");
	}
	pid_t pid = fork();
	if (pid == -1)
	{
		error("fork() failed", "CgiHandler::runCgi");
	}

	if(pid == 0)
	{
		if (dup2(pipefd[WRITE_FD], STDOUT_FILENO) == -1 || dup2(pipefd[WRITE_FD], STDERR_FILENO) == -1)
		{
			error("dup2() failed", "CgiHandler::runCgi");
		}
		close(pipefd[READ_FD]);
		close(pipefd[WRITE_FD]);

		std::string cgi_path = getCgiPath(req.getUri(), req_config);
		// char cwd[PATH_MAX];
		// std::cout << "PWD: " << getcwd(cwd, sizeof(cwd)) << std::endl;

		char **args = makeArgs(req);
		char **envp = makeEnvs(req);

		if(execve(cgi_path.c_str(), args, envp) == -1)
		{
			error("Failed to execute CGI script", "CgiHandler::runCgi");
		}
		exit(EXIT_FAILURE);
	}
	else
	{
		// read child's output
		close(pipefd[WRITE_FD]);
		char buf[4096];
		ssize_t n;
		while ((n = read(pipefd[READ_FD], buf, sizeof(buf))) > 0)
		{
			output.append(buf, n);
		}
		close(pipefd[READ_FD]);
		waitpid(pid, NULL, 0);
	}
	//maybe i need to parse the output here and set the response

	return makeResponse(output);
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
	env_vars["SERVER_NAME"] = headers["Host"].c_str();
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
	char **envp = NULL;
	envp = new char*[env_vars.size() + 1];
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
	size_t last_slash = script_filename.find_last_of('/');

	if (last_slash != std::string::npos)
	{
		script_filename = script_filename.substr(last_slash + 1);
	}
	std::string scrip_path = req_config.root + "/" +script_filename;

	args[0] = strdup(cgi_path.c_str());
	args[1] = strdup(scrip_path.c_str());
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
	std::map<std::string, std::string>::const_iterator it = req_config.cgi_pass.find(extension);
	if (it != req_config.cgi_pass.end())
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

HttpResponse CgiHandler::makeResponse(const std::string& cgi_output)
{
	HttpResponse res;
	size_t header_end = cgi_output.find("\r\n\r\n");
	if (header_end == std::string::npos)
	{
		res.setStatus(INTERNAL_SERVER_ERROR);
		res.setBody("Malformed CGI response: Missing header-body separator.");
		return res;
	}

	std::string header_section = cgi_output.substr(0, header_end);
	std::string body_section = cgi_output.substr(header_end + 4);

	std::istringstream header_stream(header_section);
	std::string line;
	bool status_set = false;

	while (std::getline(header_stream, line))
	{
		size_t colon_pos = line.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);
			value.erase(0, value.find_first_not_of(" "));

			if (key == "Status")
			{
				int status_code = atoi(value.c_str());
				res.setStatus(static_cast<e_status_code>(status_code));
				status_set = true;
			}
			else if(key == "Content-Type")
			{
				res.setMimeType(value);
			}
			else
			{
				res.setHeader(key, value);
			}
		}
	}
	if (!status_set)
	{
		res.setStatus(OK);
	}
	res.setBody(body_section);
	return res;
}