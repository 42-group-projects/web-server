#pragma once

#include <iostream>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>

#include "defaultConfigs.hpp"
#include "../src/errorHandling/ErrorWarning.hpp"

class FileSystem;
class SafePath;

struct LocationConfig
{
	std::string location;

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
};

class ServerConfig
{
public:
	ServerConfig();
	void initServerConfig(int argc, char **argv);

	const std::string& getRoot() const;
	const std::vector<std::pair<std::string, int> >& getListen() const;
	const std::map<int, std::string>& getErrorPages() const;
	size_t getClientMaxBodySize() const;
	const std::map<std::string, LocationConfig>& getLocations() const;
	const std::string& getServerName() const;

	LocationConfig operator[](const SafePath& safePath) const;
	LocationConfig operator[](const std::string& location) const;
	LocationConfig operator[](const FileSystem& file) const;
	LocationConfig getLocationConfigOrDefault(std::map<std::string, LocationConfig>::const_iterator it) const;

private:
	std::string root;
	std::vector<std::pair<std::string, int> > listen;
	std::map<int, std::string> error_pages;
	size_t client_max_body_size;
	std::map<std::string, LocationConfig> locations;
	std::string server_name;

	void setupDefaultConfig();

	std::vector<std::string> loadConfigFile(const std::string& path);

	void parseConfig(std::vector<std::string>& rawConfig);
	void checkConfig();

	void setRoot(std::vector<std::string>& tokens);
	void setListen(std::vector<std::string>& tokens);
	void setServerName(std::vector<std::string>& tokens);
	void setErrorPage(std::vector<std::string>& tokens);
	void setClientMaxBodySize(std::vector<std::string>& tokens);
	void setLocation(std::vector<std::string>& tokens, std::vector<std::string>& configVect, size_t *i);

	LocationConfig fillLocationConfig(std::vector<std::string>& locationVect, std::string location);
	LocationConfig setupDefaultLocationConfig(const std::string& location);

	void setLocationMethods(std::vector<std::string>& tokens, LocationConfig& c);
	void setLocationRoot(std::vector<std::string>& tokens, LocationConfig& c);
	void setLocationIndex(std::vector<std::string>& tokens, LocationConfig& c);
	void setOnOffDirective(std::vector<std::string>& tokens, bool& field);
	void setLocationUploadPath(std::vector<std::string>& tokens, LocationConfig& c);
	void setLocationRedirect(std::vector<std::string>& tokens, LocationConfig& c);
	void setLocationCgi(std::vector<std::string>& tokens, LocationConfig& c);

	std::vector<std::string>& normalizeConfig(std::vector<std::string>& rawConfig);
	void removeComments(std::vector<std::string>& rawConfig, int i);
	void trimCollapse(std::vector<std::string>& rawConfig, int i);
	void splitBrackets(std::vector<std::string>& rawConfig, int i);
};

std::vector<std::string> splitWords(const std::string &line);
bool isValidIPv4(const std::string &ip);
bool isValidListenString(const std::string &s);
bool isNumber(const std::string &s);
bool isValidStatusCode(const std::string &s);
bool endsWithHtml(const std::string &s);
bool isValidRedirectTarget(const std::string& target);

void directiveError(std::string dir);
void argumentError(std::string arg, std::string dir);
void missingArgument(std::string dir);

std::ostream& operator<<(std::ostream& os, const LocationConfig& loc);
std::ostream& operator<<(std::ostream& os, const ServerConfig& server);