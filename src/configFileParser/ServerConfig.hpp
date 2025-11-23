#pragma once

#include <vector>
#include <map>

#include "../../include/defaultConfigs.hpp"
#include "../fileSystem/safePath/SafePath.hpp"
#include "./ServerBlocks.hpp"
#include "./BlocksParser.hpp"

#define COLOR_RESET		"\033[0m"
#define COLOR_SERVER	"\033[1;36m"	// Cyan
#define COLOR_SUBLABEL	"\033[1;33m"	// Yellow
#define COLOR_VALUE		"\033[0;37m"	// Light gray
#define COLOR_SECTION	"\033[1;35m"	// Magenta
#define COLOR_LABEL		"\033[0;32m"	// Green

typedef struct t_request_config
{
	SafePath safePath;
	std::string root;
	std::map<int, std::string> error_pages;
	size_t client_max_body_size;
	std::vector<std::string >server_name;

	std::string location;

	bool getAllowed;
	bool postAllowed;
	bool deleteAllowed;

	std::string index;
	bool autoindex;

	bool redirect_enabled;
	std::string redirect_url;
	int redirect_code;

	std::string upload_store;

	std::map<std::string, std::string> cgi_pass;
}	s_request_config;

class ServerConfig
{
public:
	ServerConfig();
	void initServerConfig(int argc, char **argv);

	const std::vector<std::pair<std::string, int> >& getAllListen() const;
	std::vector<t_server_config>& getConfig();
	t_request_config getRequestConfig(const std::string &serverName, const std::string& ip, int port, const std::string &requestedPath) const;
	const std::vector<t_server_config>& getConfiguration() const;

private:
	std::string filePath;
	std::vector<t_server_config> configuration;
	std::vector<std::pair<std::string, int> > allListen;
};

std::ostream& operator<<(std::ostream& os, const ServerConfig& config);
std::ostream& operator<<(std::ostream& os, const t_request_config& reqConf);
