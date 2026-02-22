#pragma once
#include "../../../include/imports.hpp"

struct t_server_config;
struct t_request_config;

class SafePath
{
private:
	std::string requestedPath;
	std::string fullPath;
	std::string location;

public:
	SafePath();
	SafePath(const std::string& path, const t_server_config* serverConf);
	SafePath(const std::string& requestedPath, const t_request_config& req_conf, bool useServerRoot);
	SafePath& operator=(const SafePath& other);

	void setFullPath(const std::string& path);

	std::vector<std::string> splitPath(const std::string &path);
	std::string getRequestedPath() const;
	std::string getFullPath() const;
	std::string getLocation() const;
	operator std::string() const;
};

std::ostream& operator<<(std::ostream& os, const SafePath& sp);

