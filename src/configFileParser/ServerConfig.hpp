#pragma once

#include <vector>
#include <map>

#include "../../include/defaultConfigs.hpp"
#include "../fileSystem/safePath/SafePath.hpp"
#include "./ServerBlocks.hpp"

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

	std::string upload_store;
	size_t client_max_body_size;

	std::map<std::string, std::string> cgi_pass;
	std::map<int, std::string> error_pages;
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

	std::vector<std::pair<std::string, int> >& getAllListen();
	std::vector<t_server_config>& getConfig();
	t_request_config getRequestConfig(const std::string &serverName, const std::string& ip, int port, const std::string &requestedPath);

private:
	std::string filePath;
	std::vector<t_server_config> configuration;
	std::vector<std::pair<std::string, int> > allListen;

	t_server_config setServerConfig(const t_server_block& serverBlock);
	std::string setRoot(const t_directive& directive);
	std::pair<std::string, int> setListen(const t_directive& directive);
	std::vector<std::string> setServerName(const t_directive& directive);
	void setErrorPage(const t_directive& directive, std::map<int, std::string>& errorPages);
	size_t setClientMaxBodySize(const t_directive& directive);

	void setLocation(const t_location_block& locBlock, std::map<std::string, t_location_config>& locations, const std::string& root);
	t_location_config setupDefaultLocationConfig(const std::string& location, const std::string& root);
	void setMethods(const t_directive& directive, t_location_config& conf);
	void setIndex(const t_directive& directive, t_location_config& conf);
	void setOnOffDirective(const t_directive& directive, bool& field);
	void setUploadPath(const t_directive& directive, t_location_config& conf);
	void setRedirect(const t_directive& directive, t_location_config& conf);
	void setCgi(const t_directive& directive, t_location_config& conf);
	friend std::ostream& operator<<(std::ostream& os, const ServerConfig& config);
};
