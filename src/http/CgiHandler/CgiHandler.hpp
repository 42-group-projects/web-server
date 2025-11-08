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
	    // Helper methods
		char **makeEnvs(const HttpRequest& req);
		char **makeArgs(const HttpRequest& req);
		std::string getQuaryString(const std::string& uri);
		std::string getCgiPath(std::string uri, const t_request_config& req_config);
		std::string getExtention(const std::string& uri);
		HttpResponse makeResponse(const std::string& cgi_output);

	public:
	    CgiHandler(const t_request_config& req_config);
	    ~CgiHandler();

		bool isCgiRequest(const HttpRequest& req);
		HttpResponse runCgi(const HttpRequest& req);
};
