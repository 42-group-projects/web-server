#pragma once

// #include <iostream>
// #include <cctype>
// #include <algorithm>
// #include <stdexcept>
// #include <sstream>
// #include <fstream>
#include <vector>
#include <map>

#include "defaultConfigs.hpp"
// #include "../src/errorHandling/ErrorWarning.hpp"
#include "./ServerBlocks.hpp"

// class FileSystem;
// class SafePath;

typedef struct t_location_config
{
	std::string location;

	bool exact;
	bool getAllowed;
	bool postAllowed;
	bool deleteAllowed;
	std::string root;
	std::string index;
	bool autoindex;

	bool redirect_enabled;
	std::string redirect_url;
	int redirect_code;

	bool upload_enabled;
	std::string upload_store;

	std::map<std::string, std::string> cgi_pass;
}	s_location_config;

typedef struct t_server_config
{
	std::string root;
	std::vector<std::pair<std::string, int> > listen;
	std::map<int, std::string> error_pages;
	size_t client_max_body_size;
	std::map<std::string, t_location_config> locations;
	std::vector<std::string >server_name;
}	s_server_config;

class ServerConfig
{
public:
	ServerConfig(int argc, char **argv);

private:
	std::vector<t_server_config> configuration;

	t_server_config setServerConfig(const t_server_block& serverBlock);
	void setRoot(std::vector<std::string>& tokens);
};
