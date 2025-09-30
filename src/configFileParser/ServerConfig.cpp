#include "ServerConfig.hpp"
#include "defaultConfigs.hpp"
#include "../src/errorHandling/ErrorWarning.hpp"
#include <iostream>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <fstream>

const std::string& ServerConfig::getRoot() const { return root; }
const std::vector<std::pair<std::string, int> >& ServerConfig::getListen() const { return listen; }
const std::map<int, std::string>& ServerConfig::getErrorPages() const { return error_pages; }
size_t ServerConfig::getClientMaxBodySize() const { return client_max_body_size; }
const std::string& ServerConfig::getServerName() const { return server_name; }
const std::map<std::string, LocationConfig>& ServerConfig::getLocations() const { return locations; }

LocationConfig ServerConfig::operator[](const std::string& path) const
{
	std::map<std::string, LocationConfig>::const_iterator it = locations.find(path);

	if (it != locations.end())
		return it->second;

	LocationConfig defaultConfig;
	defaultConfig.path = path;
	defaultConfig.getAllowed = GET_ALLOWED;
	defaultConfig.postAllowed = POST_ALLOWED;
	defaultConfig.deleteAllowed = DELETE_ALLOWED;
	defaultConfig.root = root;
	defaultConfig.index = INDEX;
	defaultConfig.autoindex = AUTOINDEX;
	defaultConfig.redirect_enabled = REDIRECT_ENABLED;
	defaultConfig.redirect_url = REDIRECT_URL;
	defaultConfig.redirect_code = 0;
	defaultConfig.upload_enabled = UPLOAD_ENABLED;
	defaultConfig.upload_store = UPLOAD_STORE;
	return defaultConfig;
}

ServerConfig::ServerConfig() {}

void ServerConfig::initServerConfig(int argc, char **argv)
{
	std::vector<std::string> rawConfig;
	setupDefaultConfig();

	if (argc == 1)
	{
		warning("No file provided. Using default file", "Configuration file");
		rawConfig = loadConfigFile(DEFAULT_CONF_FILE);
	}
	else
	{
		std::string arg(argv[1]);
		size_t ext = arg.rfind(".conf");

		if (ext != std::string::npos && ext + 5 == arg.size())
			rawConfig = loadConfigFile(arg);
		else
		{
			warning("Invalid file. Using default file", "Configuration file");
			rawConfig = loadConfigFile(DEFAULT_CONF_FILE);
		}
	}

	parseConfig(rawConfig);
}

void ServerConfig::setupDefaultConfig()
{
	root = "";
	listen.clear();
	server_name = SERVER_NAME;
	error_pages.clear();
	client_max_body_size = CLIENT_MAX_BODY_SIZE;
}

std::vector<std::string> ServerConfig::loadConfigFile(const std::string& path)
{
	std::ifstream file(path.c_str());

	if (!file)
	{
		if (path == DEFAULT_CONF_FILE)
			error("file '" + std::string(DEFAULT_CONF_FILE) + "' not found. Server will not start.", "Configuration file");
		else
		{
			warning("Couldn't open file. Using default file", "Configuration file");
			return loadConfigFile(DEFAULT_CONF_FILE);
		}
	}

	std::vector<std::string> lines;
	std::string line;

	while (std::getline(file, line))
		lines.push_back(line);

	return lines;
}

void ServerConfig::parseConfig(std::vector<std::string>& rawConfig)
{
	std::vector<std::string> configVect = normalizeConfig(rawConfig);

	for (size_t i = 0; i < configVect.size(); i++)
	{
		std::vector<std::string> tokens = splitWords(configVect[i]);

		if (tokens[0] == "root")
			setRoot(tokens);
		else if (tokens[0] == "listen")
			setListen(tokens);
		else if (tokens[0] == "server_name")
			setServerName(tokens);
		else if (tokens[0] == "error_page")
			setErrorPage(tokens);
		else if (tokens[0] == "client_max_body_size")
			setClientMaxBodySize(tokens);
		else if (tokens[0] == "location")
			setLocation(tokens, configVect, &i);
		else
			directiveError(tokens[0]);
	}

	checkConfig();
}

void ServerConfig::checkConfig()
{
	if (root.empty())
		error("Missing root directive.", "Configuration file");

	if (listen.empty())
		error("Missing listen directive.", "Configuration file");

	for (std::map<std::string, LocationConfig>::iterator it = locations.begin(); it != locations.end(); ++it)
	{
		const std::string& path = it->first;
		LocationConfig& config = it->second;

		if (config.root.empty())
			config.root = root;

		if (config.upload_enabled)
			if (config.upload_store.empty())
				error("Missing upload path directive for location " + path, "Configuration file");
	}
}

void ServerConfig::setRoot(std::vector<std::string>& tokens)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	if (tokens[1][0] != '/')
		error(tokens[1] + " is not an absolute path.", "Configuration file");

	root = tokens[1];
}

void ServerConfig::setListen(std::vector<std::string>& tokens)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (!isValidListenString(tokens[1]))
		argumentError(tokens[1], tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	size_t colon = tokens[1].find(':');
	std::string ip = tokens[1].substr(0, colon);
	std::string portStr = tokens[1].substr(colon + 1);
	int port;
	std::stringstream ss(portStr);

	if (!(ss >> port) || !ss.eof())
		argumentError(portStr, tokens[0]);

	listen.push_back(std::make_pair(ip, port));
}

void ServerConfig::setServerName(std::vector<std::string>& tokens)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	server_name = tokens[1];
}

void ServerConfig::setErrorPage(std::vector<std::string>& tokens)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (!isValidStatusCode(tokens[1]))
		argumentError(tokens[1], tokens[0]);

	if (!endsWithHtml(tokens[2]))
		argumentError(tokens[2], tokens[0]);

	if (tokens[2][0] != '/')
		error(tokens[2] + " is not an absolute path.", "Configuration file");

	if (tokens.size() > 3)
		argumentError(tokens[3], tokens[0]);

	int code;
	std::stringstream ss(tokens[1]);

	if (!(ss >> code) || !ss.eof())
		argumentError(tokens[1], tokens[0]);

	error_pages[code] = tokens[2];
}

void ServerConfig::setClientMaxBodySize(std::vector<std::string>& tokens)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	char last = std::tolower(tokens[1][tokens[1].size() - 1]);
	size_t multiplier = 1;

	if (last == 'k')
		multiplier = 1024;
	else if (last == 'm')
		multiplier = 1024 * 1024;
	else if (last == 'g')
		multiplier = 1024 * 1024 * 1024;

	if (multiplier != 1)
		tokens[1] = tokens[1].substr(0, tokens[1].size() - 1);

	for (size_t i = 0; i < tokens[1].size(); ++i)
	{
		if (!std::isdigit(tokens[1][i]))
			argumentError(tokens[1], tokens[0]);
	}

	unsigned long long value;
	std::stringstream ss(tokens[1]);

	if (!(ss >> value) || !ss.eof())
		argumentError(tokens[1], tokens[0]);

	client_max_body_size = static_cast<size_t>(value * multiplier);
}

void ServerConfig::setLocation(std::vector<std::string>& tokens, std::vector<std::string>& configVect, size_t *i)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	if (configVect[(*i) + 1][0] != '{')
		error("missing '{' in directive " + tokens[0] + " " + tokens[1] + ".", "Configuration file");

	size_t j;

	for (j = *i; j < configVect.size(); ++j)
	{
		if (configVect[j][0] == '}')
			break;
	}

	if (j >= configVect.size())
		error("missing '}' in directive " + tokens[0] + " " + tokens[1] + ".", "Configuration file");

	std::vector<std::string> locationVect(configVect.begin() + *i + 2, configVect.begin() + j);
	*i = j;
	LocationConfig c = fillLocationConfig(locationVect, tokens[1]);
	locations[c.path] = c;
}

LocationConfig ServerConfig::fillLocationConfig(std::vector<std::string>& locationVect, std::string location)
{
	if (location[0] != '/')
		error("location path '" + location + "' must start with '/'", "Configuration file");

	LocationConfig c = setupDefaultLocationConfig(location);
	c.path = location;

	for (size_t i = 0; i < locationVect.size(); i++)
	{
		std::vector<std::string> tokens = splitWords(locationVect[i]);

		if (tokens.empty())
			continue;

		if (tokens[0] == "methods")
			setLocationMethods(tokens, c);
		else if (tokens[0] == "root")
			setLocationRoot(tokens, c);
		else if (tokens[0] == "index")
			setLocationIndex(tokens, c);
		else if (tokens[0] == "autoindex")
			setOnOffDirective(tokens, c.autoindex);
		else if (tokens[0] == "upload_enabled")
			setOnOffDirective(tokens, c.upload_enabled);
		else if (tokens[0] == "upload_path")
			setLocationUploadPath(tokens, c);
		else if (tokens[0] == "return")
			setLocationRedirect(tokens, c);
		else if (tokens[0] == "cgi")
			setLocationCgi(tokens, c);
		else
			directiveError(tokens[0]);
	}

	return c;
}

LocationConfig ServerConfig::setupDefaultLocationConfig(const std::string& location)
{
	LocationConfig c;
	c.root = "";
	c.path = location;
	c.getAllowed = GET_ALLOWED;
	c.postAllowed = POST_ALLOWED;
	c.deleteAllowed = DELETE_ALLOWED;
	c.index = INDEX;
	c.autoindex = AUTOINDEX;
	c.redirect_enabled = REDIRECT_ENABLED;
	c.redirect_url = REDIRECT_URL;
	c.redirect_code = 0;
	c.upload_enabled = UPLOAD_ENABLED;
	c.upload_store = "";
	c.cgi_pass.clear();
	return c;
}

void ServerConfig::setLocationMethods(std::vector<std::string>& tokens, LocationConfig& c)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 4)
		argumentError(tokens[4], tokens[0]);

	c.getAllowed = false;
	c.postAllowed = false;
	c.deleteAllowed = false;

	for (size_t i = 1; i < tokens.size(); i++)
	{
		if (tokens[i] == "GET")
			c.getAllowed = true;
		else if (tokens[i] == "POST")
			c.postAllowed = true;
		else if (tokens[i] == "DELETE")
			c.deleteAllowed = true;
		else
			argumentError(tokens[i], tokens[0]);
	}
}

void ServerConfig::setLocationRoot(std::vector<std::string>& tokens, LocationConfig& c)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	if (tokens[1][0] != '/')
		error(tokens[1] + " is not an absolute path.", "Configuration file");

	c.root = tokens[1];
}

void ServerConfig::setLocationIndex(std::vector<std::string>& tokens, LocationConfig& c)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	c.index = tokens[1];
}

void ServerConfig::setOnOffDirective(std::vector<std::string>& tokens, bool& field)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	if (tokens[1] == "on")
		field = true;
	else if (tokens[1] == "off")
		field = false;
	else
		argumentError(tokens[1], tokens[0]);
}

void ServerConfig::setLocationUploadPath(std::vector<std::string>& tokens, LocationConfig& c)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	if (tokens[1][0] != '/')
		error(tokens[1] + " is not an absolute path.", "Configuration file");

	c.upload_store = tokens[1];
}

void ServerConfig::setLocationRedirect(std::vector<std::string>& tokens, LocationConfig& c)
{
	if (tokens.size() == 2)
		missingArgument(tokens[0]);

	if (tokens.size() > 3)
		argumentError(tokens[3], tokens[0]);

	if (!isValidStatusCode(tokens[1]))
		argumentError(tokens[1], tokens[0]);

	if (!isValidRedirectTarget(tokens[2]))
		error(tokens[2] + " is not a valid redirect target.", "Configuration file");

	c.redirect_url = tokens[2];
	int code;
	std::stringstream ss(tokens[1]);

	if (!(ss >> code) || !ss.eof())
		argumentError(tokens[1], tokens[0]);

	c.redirect_code = code;
	c.redirect_enabled = true;
}

void ServerConfig::setLocationCgi(std::vector<std::string>& tokens, LocationConfig& c)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 3)
		argumentError(tokens[3], tokens[0]);

	if (!(tokens[1].size() > 1 && tokens[1][0] == '.'))
		error(tokens[1] + " is not a valid cgi extension.", "Configuration file");

	if (tokens[2][0] != '/')
		error(tokens[2] + " is not an absolute path.", "Configuration file");

	c.cgi_pass[tokens[1]] = tokens[2];
}

std::vector<std::string>& ServerConfig::normalizeConfig(std::vector<std::string>& rawConfig)
{
	for (int i = rawConfig.size() - 1; i >= 0; i--)
	{
		if (rawConfig[i].empty())
		{
			rawConfig.erase(rawConfig.begin() + i);
			continue;
		}

		removeComments(rawConfig, i);
		trimCollapse(rawConfig, i);
		splitBrackets(rawConfig, i);
	}

	return rawConfig;
}

void ServerConfig::removeComments(std::vector<std::string>& rawConfig, int i)
{
	if (rawConfig[i][0] == '#')
	{
		rawConfig.erase(rawConfig.begin() + i);
		return ;
	}

	size_t pos = rawConfig[i].find('#');

	if (pos != std::string::npos)
		rawConfig[i] = rawConfig[i].substr(0, pos);
}

void ServerConfig::trimCollapse(std::vector<std::string>& rawConfig, int i)
{
	size_t start = 0;
	size_t end = rawConfig[i].size() - 1;

	while (start <= end && (rawConfig[i][start] == ' ' || rawConfig[i][start] == '\t'))
		start++;

	while (end >= start && (rawConfig[i][end] == ' ' || rawConfig[i][end] == '\t'))
		end--;

	if (start > end)
	{
		rawConfig.erase(rawConfig.begin() + i);
		return;
	}

	rawConfig[i] = rawConfig[i].substr(start, end - start + 1);
}

void ServerConfig::splitBrackets(std::vector<std::string>& rawConfig, int i)
{
	int lastCharIndex = rawConfig[i].size() - 1;

	if (!lastCharIndex)
		return;

	char lastChar = rawConfig[i][lastCharIndex];

	if ((lastChar == '{' || lastChar == '}'))
	{
		rawConfig[i] = rawConfig[i].substr(0, rawConfig[i].size() - 1);
		rawConfig.insert(rawConfig.begin() + i + 1, std::string(1, lastChar));
	}
}

std::vector<std::string> splitWords(const std::string &line)
{
	std::vector<std::string> words;
	std::istringstream iss(line);
	std::string word;

	while (iss >> word)
		words.push_back(word);

	return words;
}

bool isValidIPv4(const std::string &ip)
{
	std::stringstream ss(ip);
	std::string segment;
	int count = 0;

	while (std::getline(ss, segment, '.'))
	{
		if (segment.empty() || segment.size() > 3)
			return false;

		for (std::string::size_type i = 0; i < segment.size(); ++i)
			if (!std::isdigit(segment[i]))
				return false;

		int num = std::atoi(segment.c_str());

		if (num < 0 || num > 255)
			return false;

		count++;
	}

	return count == 4;
}

bool isValidListenString(const std::string &s)
{
	std::string::size_type colon = s.find(':');

	if (colon == std::string::npos)
		return false;

	std::string ip = s.substr(0, colon);
	std::string portStr = s.substr(colon + 1);

	if (!isValidIPv4(ip))
		return false;

	if (portStr.empty())
		return false;

	for (std::string::size_type i = 0; i < portStr.size(); ++i)
		if (!std::isdigit(portStr[i]))
			return false;

	int port = std::atoi(portStr.c_str());

	if (port < 1 || port > 65535)
		return false;

	return true;
}

bool isNumber(const std::string &s)
{
	if (s.empty())
		return false;

	for (std::string::size_type i = 0; i < s.size(); ++i)
		if (!std::isdigit(s[i]))
			return false;

	return true;
}

bool isValidStatusCode(const std::string &s)
{
	if (!isNumber(s))
		return false;

	int code = std::atoi(s.c_str());
	static const int codes[] =
	{
		200, 201, 202, 204, 205, 206, 207, 208, 226,
		300, 301, 302, 303, 304, 305, 306, 307, 308,
		400, 401, 402, 403, 404, 405, 406, 407, 408, 409,
		410, 411, 412, 413, 414, 415, 416, 417, 418, 421,
		422, 423, 424, 425, 426, 428, 429, 431, 451,
		500, 501, 502, 503, 504, 505, 506, 507, 508, 510, 511
	};

	for (size_t i = 0; i < sizeof(codes) / sizeof(codes[0]); ++i)
	{
		if (code == codes[i])
			return true;
	}

	return false;
}

bool endsWithHtml(const std::string &s)
{
	if (s.size() < 5)
		return false;

	return s.substr(s.size() - 5) == ".html";
}

bool isValidRedirectTarget(const std::string& target)
{
	if (target.empty())
		return false;

	if (target[0] == '/')
		return true;

	if (target.find("http://") == 0 || target.find("https://") == 0)
		return true;

	return false;
}

void directiveError(std::string dir)
{
	error("unknown directive '" + dir + "'.", "Configuration file");
}

void argumentError(std::string arg, std::string dir)
{
	error("invalid argument '" + arg + "' for directive '" + dir + "'.", "Configuration file");
}

void missingArgument(std::string dir)
{
	error("missing argument for directive '" + dir + "'.", "Configuration file");
}
