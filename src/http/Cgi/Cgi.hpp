#pragma once
#include "../../../include/imports.hpp"
#include "../../../include/enums.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../../configFileParser/ServerConfig.hpp"
#include "../../fileSystem/FileSystem.hpp"

class CgiHandler
{
private:
    const LocationConfig& location_config;
    // Helper methods
	void setEnvs(const HttpRequest& req, const LocationConfig& config);
	std::string getQuaryString(const std::string& uri);
	std::string getCgiPath(std::string uri, const LocationConfig& config);
	std::string getExtention(const std::string& uri);
public:

    CgiHandler(const LocationConfig& config);
    ~CgiHandler();

	bool isCgiRequest(const HttpRequest& req);
	HttpRequest runCgi(const HttpRequest& req);
};
