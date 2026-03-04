#include "CgiHandler.hpp"
#include "../utils.hpp"
#include "../../../include/imports.hpp"

CgiHandler::CgiHandler(const t_request_config& req_config)
	: req_config(req_config), cgi_interpreter(""), script_name(""), path_info("") {}

CgiHandler::~CgiHandler() {}

int CgiHandler::runCgi(const HttpRequest& req, pid_t& childPid, int& inFd)
{
    HttpRequest request = req;

    int out_pipe[2];
    int in_pipe[2];

    bool is_post = (req.getMethod() == POST);

    if (pipe(out_pipe) == -1) return -1;
    if (is_post && pipe(in_pipe) == -1) {
        close(out_pipe[0]); close(out_pipe[1]);
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        close(out_pipe[0]); close(out_pipe[1]);
        if (is_post) { close(in_pipe[0]); close(in_pipe[1]); }
        return -1;
    }

    if (pid == 0)
    {
        // --- CHILD ---
        detectCgiType(req);
        char **args = makeArgs(req);
        char **envp = makeEnvs(req);

        // Redirect stdout/stderr to out_pipe
        dup2(out_pipe[WRITE_FD], STDOUT_FILENO);
        dup2(out_pipe[WRITE_FD], STDERR_FILENO);

        close(out_pipe[READ_FD]);
        close(out_pipe[WRITE_FD]);

        if (is_post) {
            dup2(in_pipe[READ_FD], STDIN_FILENO);
            close(in_pipe[READ_FD]); close(in_pipe[WRITE_FD]);
        }

        std::string relative_path = (req_config.root[0] == '/') ? ("." + req_config.root) : ("./" + req_config.root);
        chdir(relative_path.c_str());

        execve(cgi_interpreter.c_str(), args, envp);

        // Exec failed: write 502 to stdout
        std::string code = "502\n";
        write(STDOUT_FILENO, code.c_str(), code.size());
        _exit(1);
    }
    else
    {
	// --- PARENT ---
	childPid = pid;
		std::cerr << "RAW BODY: [" << req.getBody() << "]" << std::endl;
	// Close write end for parent reading
	close(out_pipe[WRITE_FD]);

	if (is_post) {
		close(in_pipe[READ_FD]);

		int writeFd = in_pipe[WRITE_FD];

		const std::string &body = req.getBody();
		size_t totalWritten = 0;

		while (totalWritten < body.size()) {
			ssize_t n = write(writeFd,
							body.c_str() + totalWritten,
							body.size() - totalWritten);

			if (n <= 0) {
				break; // write error
			}
			totalWritten += n;
		}

		// VERY IMPORTANT: signal EOF to CGI
		close(writeFd);

		inFd = -1;  // nothing left to manage
		}
		else {
			inFd = -1;
		}

        // Make output pipe non-blocking
        fcntl(out_pipe[READ_FD], F_SETFL, fcntl(out_pipe[READ_FD], F_GETFL, 0) | O_NONBLOCK);

        // Return fd for asynchronous reading
        return out_pipe[READ_FD];
    }
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

	// After chdir() to req_config.root, we only need the script filename
std::string script_for_exec = "." + script_name;
	char **args;
	args = new char*[3];
	args[0] = strdup(cgi_interpreter.c_str());
	args[1] = strdup(script_for_exec.c_str());
	args[2] = NULL;
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
	env_vars["QUERY_STRING"] = sanitizeQueryString(req.getQueryString());

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

bool isWhiteListed(char c)
{
	return ( (c >= 'a' && c <= 'z') ||
			 (c >= 'A' && c <= 'Z') ||
			 (c >= '0' && c <= '9') ||
			 c == '-' || c == '_' || c == '.' ||
			 c == '=' || c == '&' || c == '%' );
}

std::string CgiHandler::sanitizeQueryString(const std::string& query)
{
	std::string sanitized;
	sanitized.reserve(query.size());

	for (size_t i = 0; i < query.size(); ++i)
	{
		char c = query[i];
		if (isWhiteListed(c))
		{
		    sanitized += c;
		}
		else
		{
			char buf[4];
			std::sprintf(buf, "%%%02X", (unsigned char)c);
			sanitized += buf;
		}
	}
	return sanitized;
}
