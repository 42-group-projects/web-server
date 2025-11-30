#pragma once
#include "../../../include/imports.hpp"
#include "../../../include/enums.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../../configFileParser/ServerConfig.hpp"
#include "../../fileSystem/FileSystem.hpp"
#include "../HttpResponse/HttpResponse.hpp"

#define READ_FD  0
#define WRITE_FD 1

class CgiHandler
{
private:
	t_request_config req_config;

	// Helpers
	void detectCgiType(const HttpRequest& req);
	char **makeEnvs(const HttpRequest& req);
	char **makeArgs(const HttpRequest& req);
	std::string getQueryString(const HttpRequest& req);
	std::string getExtension(const std::string& uri);

	// CGI state
	std::string cgi_interpreter;
	std::string script_name;
	std::string path_info;

public:
	CgiHandler(const t_request_config& req_config);
	~CgiHandler();

	HttpResponse runCgi(const HttpRequest& req);
};
