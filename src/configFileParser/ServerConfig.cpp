#include "ServerConfig.hpp"
#include "error_messages.hpp"
#include "arg_validity_checks.hpp"
#include "../../src/errorHandling/ErrorWarning.hpp"

#include <sstream>
#include <cstdlib>

ServerConfig::ServerConfig() {}

void ServerConfig::initServerConfig(int argc, char **argv)
{
	defaultServerSet = false;
	TokenizeFile tokenizer(argc, argv);
	filePath = tokenizer.getFilePath();
	ServerBlocks serverBlocks(tokenizer);
	const std::vector<t_server_block> &servers = serverBlocks.getServerBlocks();

	for (size_t i = 0; i < servers.size(); i++)
		configuration.push_back(setServerConfig(servers[i]));

	if (hasDuplicateServerBlocks())
		error("Error: duplicate server_name:listen combination detected", "Configuration file");
}



const std::vector<std::pair<std::string, int> >& ServerConfig::getAllListen() const {return allListen;}
std::vector<t_server_config>& ServerConfig::getConfig() {return configuration;}
const std::vector<t_server_config>& ServerConfig::getConfiguration() const { return configuration; }


t_request_config ServerConfig::getRequestConfig(const std::string &serverName, const std::string& ip, int port, const std::string &requestedPath) const
{
	std::pair<std::string, int> ipPort(ip, port);
	const t_server_config *sconf = NULL;
	std::vector<const t_server_config*> exactMatches;
	std::vector<const t_server_config*> catchAll;

	for (size_t i = 0; i < configuration.size(); ++i)
	{
		for (size_t j = 0; j < configuration[i].listen.size(); ++j)
		{
			const std::pair<std::string, int> &listen = configuration[i].listen[j];

			if (listen == ipPort)
				exactMatches.push_back(&configuration[i]);
			else if (listen.first == "0.0.0.0" && listen.second == ipPort.second)
				catchAll.push_back(&configuration[i]);
		}
	}

	const std::vector<const t_server_config*> &servers = !exactMatches.empty() ? exactMatches : catchAll;

	for (size_t i = 0; i < servers.size() && !sconf; ++i)
	{
		const std::vector<std::string> &names = servers[i]->server_name;

		for (size_t j = 0; j < names.size(); ++j)
		{
			if (serverName == names[j])
			{
				sconf = servers[i];
				break;
			}
		}
	}

	if (!sconf)
	{
		int deflt = 0;

		for (size_t i = 0; i < servers.size(); ++i)
			if (servers[i]->defaultServer)
				deflt = i;

		sconf = servers[deflt];
	}

	SafePath sp(requestedPath, sconf);
	const t_location_config &lConf = sconf->locations.at(sp.getLocation());
	t_request_config rConf =
	{
		sp,
		lConf.root.empty() ? sconf->root : lConf.root,
		lConf.error_pages.empty() ? sconf->error_pages : lConf.error_pages,
		lConf.client_max_body_size ? lConf.client_max_body_size : sconf->client_max_body_size,
		sconf->server_name,
		sp.getLocation(),
		lConf.getAllowed,
		lConf.postAllowed,
		lConf.deleteAllowed,
		lConf.index,
		lConf.autoindex,
		lConf.redirect_enabled,
		lConf.redirect_url,
		lConf.redirect_code,
		lConf.upload_store,
		lConf.cgi_pass
	};
	return rConf;
}




t_server_config ServerConfig::setServerConfig(const t_server_block &serverBlock)
{
	t_server_config config;
	config.defaultServer = false;
	config.root = "";
	config.listen.clear();
	config.server_name.push_back(SERVER_NAME);
	config.error_pages.clear();
	config.client_max_body_size = CLIENT_MAX_BODY_SIZE;
	bool root = false;
	bool listen = false;

	for (size_t i = 0; i < serverBlock.directives.size(); i++)
	{
		t_directive directive = serverBlock.directives[i];

		if (directive.directive.str == "root")
		{
			config.root = setRoot(directive);
			root = true;
		}
		else if (directive.directive.str == "listen")
		{
			std::pair<std::string, int> list =  setListen(directive);
			config.listen.push_back(list);
			allListen.push_back(list);
			listen = true;
		}
		else if (directive.directive.str == "server_name")
			config.server_name = setServerName(directive);
		else if (directive.directive.str == "error_page")
			setErrorPage(directive, config.error_pages);
		else if (directive.directive.str == "default")
		{
			if (defaultServerSet)
				error_messages::alreadyDefined("Default server", directive.directive, filePath);

			config.defaultServer = true;
			defaultServerSet = true;
		}
		else if (directive.directive.str == "client_max_body_size")
			config.client_max_body_size = setClientMaxBodySize(directive);
		else
			error_messages::expected("directive value", directive.directive, filePath);
	}

	if (!listen)
		error_messages::missingDirective(serverBlock.directives[0].directive, "listen", filePath);

	if (!root)
		error_messages::missingDirective(serverBlock.directives[0].directive, "root", filePath);

	// bool mainLocation = false;

	for (size_t i = 0; i < serverBlock.locations.size(); i++)
	{
		// if (serverBlock.locations[i].name.str == "/")
		// 	mainLocation = true;
		setLocation(serverBlock.locations[i], config.locations, config.root);
	}

	// if (!mainLocation)
	// 	error_messages::missingLocation(serverBlock.directives[0].directive, filePath);
	return config;
}

std::string ServerConfig::setRoot(const t_directive& directive)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);
	arg_validity_checks::checkPathEndsWithSlash(directive.options[0], filePath);
	arg_validity_checks::checkSpecialCharacters(directive.options[0], filePath);
	return directive.options[0].str;
}

std::pair<std::string, int> ServerConfig::setListen(const t_directive& directive)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);
	std::string ipPort = directive.options[0].str;
	size_t colon = ipPort.find(':');

	if (colon == std::string::npos)
		ipPort = "0.0.0.0:" + ipPort;

	colon = ipPort.find(':');
	std::string ip = ipPort.substr(0, colon);
	std::string portStr = ipPort.substr(colon + 1);
	int port;
	std::stringstream ss(portStr);

	if (!(ss >> port) || !ss.eof())
		error_messages::invalidPort(directive.options[0], portStr, filePath);

	arg_validity_checks::checkIP(directive.options[0], ip, filePath);
	arg_validity_checks::checkPort(directive.options[0], port, filePath);
	return std::make_pair(ip, port);
}

std::vector<std::string> ServerConfig::setServerName(const t_directive& directive)
{
	arg_validity_checks::optionsCount(directive, 1, filePath);
	std::vector<std::string> res;

	for (size_t i = 0; i < directive.options.size(); i++)
		res.push_back(directive.options[i].str);

	return res;
}

void ServerConfig::setErrorPage(const t_directive& directive, std::map<int, std::string>& errorPages)
{
	arg_validity_checks::optionsCount(directive, 2, filePath);
	arg_validity_checks::isValidStatusCode(directive, filePath);
	arg_validity_checks::endsWithHtml(directive.options[directive.options.size() - 1], filePath);
	arg_validity_checks::checkAbsolutePath(directive.options[directive.options.size() - 1], filePath);

	for (size_t i = 0; i + 1 < directive.options.size(); ++i)
	{
		int code = std::atoi(directive.options[i].str.c_str());
		std::string path = directive.options.back().str;
		std::pair<int, std::string> p = std::make_pair(code, path);
		errorPages.insert(p);
	}
}
size_t ServerConfig::setClientMaxBodySize(const t_directive& directive)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);
	std::string option = directive.options[0].str;
	char last = std::tolower(option[option.size() - 1]);
	size_t multiplier = 1;

	if (last == 'k')
		multiplier = 1024;
	else if (last == 'm')
		multiplier = 1024 * 1024;
	else if (last == 'g')
		multiplier = 1024 * 1024 * 1024;

	if (multiplier != 1)
		option = option.substr(0, option.size() - 1);

	if (!arg_validity_checks::isNumber(option))
		error_messages::invalidClientMaxBodySize(directive.options[0], option, filePath);

	unsigned long long value;
	std::stringstream ss(option);

	if (!(ss >> value) || !ss.eof())
		error_messages::invalidClientMaxBodySize(directive.options[0], option, filePath);

	unsigned long long finalValue = value * multiplier;

	if (finalValue > CONFIG_FILE_MAX_CLIENT_BODY_SIZE)
		error_messages::invalidClientMaxBodySize(directive.options[0], option, filePath);

	return static_cast<size_t>(finalValue);
}

void ServerConfig::setLocation(const t_location_block& locBlock, std::map<std::string, t_location_config>& locations, const std::string& root)
{
	arg_validity_checks::checkAbsolutePath(locBlock.name, filePath);
	t_location_config conf;
	conf = setupDefaultLocationConfig(locBlock.name.str, root);
	conf.exact = locBlock.exact;
	bool rootDirective = false;
	bool returnDirective = false;
	bool indexDirective = false;
	bool autoIndexDirective = false;
	bool uploadDirective = false;

	for (size_t i = 0; i < locBlock.directives.size(); i++)
	{
		std::string str = locBlock.directives[i].directive.str;

		if (str == "methods")
			setMethods(locBlock.directives[i], conf);
		else if (str == "root")
		{
			rootDirective = true;

			if (returnDirective)
				error_messages::conflictingDirectives("return", locBlock.directives[i].directive, filePath);

			conf.root = setRoot(locBlock.directives[i]);
		}
		else if (str == "index")
		{
			indexDirective = true;

			if (returnDirective)
				error_messages::conflictingDirectives("return", locBlock.directives[i].directive, filePath);

			setIndex(locBlock.directives[i], conf);
		}
		else if (str == "autoindex")
		{
			autoIndexDirective = true;

			if (returnDirective)
				error_messages::conflictingDirectives("return", locBlock.directives[i].directive, filePath);

			setOnOffDirective(locBlock.directives[i], conf.autoindex);
		}
		else if (str == "upload_store")
		{
			uploadDirective = true;

			if (returnDirective)
				error_messages::conflictingDirectives("return", locBlock.directives[i].directive, filePath);

			setUploadPath(locBlock.directives[i], conf);
		}
		else if (str == "return")
		{
			returnDirective = true;

			if (rootDirective)
				error_messages::conflictingDirectives("root", locBlock.directives[i].directive, filePath);
			else if (indexDirective)
				error_messages::conflictingDirectives("index", locBlock.directives[i].directive, filePath);
			else if (autoIndexDirective)
				error_messages::conflictingDirectives("autoindex", locBlock.directives[i].directive, filePath);
			else if (uploadDirective)
				error_messages::conflictingDirectives("upload_store", locBlock.directives[i].directive, filePath);

			setRedirect(locBlock.directives[i], conf);
		}
		else if (str == "cgi_handler")
			setCgi(locBlock.directives[i], conf);
		else if (str == "error_page")
			setErrorPage(locBlock.directives[i], conf.error_pages);
		else if (str == "client_max_body_size")
			conf.client_max_body_size = setClientMaxBodySize(locBlock.directives[i]);
		else
			error_messages::expected("directive value", locBlock.directives[i].directive, filePath);
	}

	for (std::map<std::string, t_location_config>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		if (it->first == locBlock.name.str)
			error_messages::duplicateDirectives("location", locBlock.name, filePath);
	}

	locations.insert(std::make_pair(locBlock.name.str, conf));
}

t_location_config ServerConfig::setupDefaultLocationConfig(const std::string& location, const std::string& root)
{
	t_location_config conf;
	conf.root = root;
	conf.client_max_body_size = 0;
	conf.location = location;
	conf.getAllowed = GET_ALLOWED;
	conf.postAllowed = POST_ALLOWED;
	conf.deleteAllowed = DELETE_ALLOWED;
	conf.index = INDEX;
	conf.autoindex = AUTOINDEX;
	conf.redirect_enabled = REDIRECT_ENABLED;
	conf.redirect_url = REDIRECT_URL;
	conf.redirect_code = 0;
	conf.upload_store = "";
	conf.cgi_pass.clear();
	conf.error_pages.clear();
	return conf;
}

void ServerConfig::setMethods(const t_directive& directive, t_location_config& conf)
{
	arg_validity_checks::optionsCount(directive, 1, 3, filePath);
	conf.getAllowed = false;
	conf.postAllowed = false;
	conf.deleteAllowed = false;

	for (size_t i = 0; i < directive.options.size(); i++)
	{
		if (directive.options[i].str == "GET")
			conf.getAllowed = true;
		else if (directive.options[i].str == "POST")
			conf.postAllowed = true;
		else if (directive.options[i].str == "DELETE")
			conf.deleteAllowed = true;
		else
			error_messages::unknownArgument(directive.options[i], filePath);
	}
}

void ServerConfig::setIndex(const t_directive& directive, t_location_config& conf)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);
	conf.index = directive.options[0].str;
}

void ServerConfig::setOnOffDirective(const t_directive& directive, bool& field)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);

	if (directive.options[0].str == "on")
		field = true;
	else if (directive.options[0].str == "off")
		field = false;
	else
		error_messages::unknownArgument(directive.options[0], filePath);
}

void ServerConfig::setUploadPath(const t_directive& directive, t_location_config& conf)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);
	conf.upload_store = directive.options[0].str;
}

void ServerConfig::setRedirect(const t_directive& directive, t_location_config& conf)
{
	arg_validity_checks::optionsCount(directive, 2, 2, filePath);
	arg_validity_checks::isValidStatusCode(directive, filePath);
	conf.redirect_url = directive.options[1].str;
	int code;
	std::stringstream ss(directive.options[0].str);

	if (!(ss >> code) || !ss.eof())
		error_messages::invalidStatusCode(directive.options[0], filePath);

	conf.redirect_code = code;
	conf.redirect_enabled = true;
}

void ServerConfig::setCgi(const t_directive& directive, t_location_config& conf)
{
	arg_validity_checks::optionsCount(directive, 1, 2, filePath);

	if (!(directive.options[0].str.size() > 1 && directive.options[0].str[0] == '.'))
		error_messages::invalidCgiExtension(directive.options[0], filePath);

	conf.cgi_pass[directive.options[0].str] = directive.options[1].str;
}

bool ServerConfig::hasDuplicateServerBlocks() const
{
	std::set<std::string> seen; // stores "server_name:ip:port" strings

	for (size_t i = 0; i < configuration.size(); ++i)
	{
		const t_server_config &srv = configuration[i];

		for (size_t j = 0; j < srv.server_name.size(); ++j)
		{
			const std::string &name = srv.server_name[j];

			for (size_t k = 0; k < srv.listen.size(); ++k)
			{
				const std::pair<std::string, int> &listen = srv.listen[k];
				std::stringstream key;
				key << name << ":" << listen.first << ":" << listen.second;

				if (seen.count(key.str()) > 0)
					return true; // duplicate found

				seen.insert(key.str());
			}
		}
	}

	return false; // no duplicates
}


#define COLOR_RESET		"\033[0m"
#define COLOR_SERVER	"\033[1;36m"	// Cyan
#define COLOR_SUBLABEL	"\033[1;33m"	// Yellow
#define COLOR_VALUE		"\033[0;37m"	// Light gray
#define COLOR_SECTION	"\033[1;35m"	// Magenta
#define COLOR_LABEL		"\033[0;32m"	// Green

std::ostream& operator<<(std::ostream& os, const ServerConfig& config)
{
	for (size_t i = 0; i < config.getConfiguration().size(); ++i)
	{
		const t_server_config& srv = config.getConfiguration()[i];
		os << "\n" << COLOR_SERVER << "==================== SERVER " << i + 1 << " ====================" << COLOR_RESET << "\n\n";
		os << COLOR_LABEL << "Root: " << COLOR_VALUE << srv.root << "\n";
		os << COLOR_LABEL << "Listen:" << COLOR_RESET << "\n";

		for (size_t j = 0; j < srv.listen.size(); ++j)
			os << COLOR_VALUE << "  " << srv.listen[j].first << ":" << srv.listen[j].second << "\n";

		os << COLOR_LABEL << "Server names:" << COLOR_RESET << "\n";

		for (size_t j = 0; j < srv.server_name.size(); ++j)
			os << COLOR_VALUE << "  " << srv.server_name[j] << "\n";

		os << COLOR_LABEL << "Client max body size: " << COLOR_VALUE << srv.client_max_body_size << COLOR_RESET << "\n";
		os << COLOR_LABEL << "Error pages:" << COLOR_RESET << "\n";

		for (std::map<int, std::string>::const_iterator it = srv.error_pages.begin(); it != srv.error_pages.end(); ++it)
			os << COLOR_VALUE << "  " << it->first << " -> " << it->second << "\n";

		os << COLOR_SECTION << "\nLOCATIONS:" << COLOR_RESET << "\n";

		for (std::map<std::string, t_location_config>::const_iterator it = srv.locations.begin(); it != srv.locations.end(); ++it)
		{
			const t_location_config& loc = it->second;
			os << "\n  " << COLOR_SUBLABEL << "Location: " << COLOR_VALUE << loc.location
			   << (loc.exact ? " (exact)" : "") << COLOR_RESET << "\n";
			os << COLOR_LABEL << "    Methods: " << COLOR_VALUE
			   << "GET[" << loc.getAllowed << "] POST[" << loc.postAllowed << "] DELETE[" << loc.deleteAllowed << "]" << COLOR_RESET << "\n";
			os << COLOR_LABEL << "    Root: " << COLOR_VALUE << loc.root << COLOR_RESET << "\n";
			os << COLOR_LABEL << "    Index: " << COLOR_VALUE << loc.index << COLOR_RESET << "\n";
			os << COLOR_LABEL << "    Autoindex: " << COLOR_VALUE << loc.autoindex << COLOR_RESET << "\n";
			os << COLOR_LABEL << "    Redirect: " << COLOR_VALUE
			   << (loc.redirect_enabled ? "enabled" : "disabled")
			   << " -> " << loc.redirect_url << " (" << loc.redirect_code << ")" << COLOR_RESET << "\n";
			os << COLOR_LABEL << "    Upload store: " << COLOR_VALUE << loc.upload_store << COLOR_RESET << "\n";
			os << COLOR_LABEL << "    Client max body size: " << COLOR_VALUE << loc.client_max_body_size << COLOR_RESET << "\n";
			os << COLOR_LABEL << "    CGI handlers:" << COLOR_RESET << "\n";

			for (std::map<std::string, std::string>::const_iterator cit = loc.cgi_pass.begin(); cit != loc.cgi_pass.end(); ++cit)
				os << COLOR_VALUE << "      " << cit->first << " -> " << cit->second << "\n";

			os << COLOR_LABEL << "    Error pages:" << COLOR_RESET << "\n";

			for (std::map<int, std::string>::const_iterator eit = loc.error_pages.begin(); eit != loc.error_pages.end(); ++eit)
				os << COLOR_VALUE << "      " << eit->first << " -> " << eit->second << "\n";
		}

		os << "\n";
	}

	return os;
}

std::ostream& operator<<(std::ostream& os, const t_request_config& req)
{
	os << "\n" << COLOR_SERVER << "=========== REQUEST CONFIG ===========" << COLOR_RESET << "\n\n";
	os << COLOR_LABEL << "Safe path:" << COLOR_RESET << "\n";
	os << COLOR_VALUE << "  Requested: " << req.safePath.getRequestedPath() << "\n";
	os << COLOR_VALUE << "  Full:      " << req.safePath.getFullPath() << "\n";
	os << COLOR_VALUE << "  Location:  " << req.safePath.getLocation() << "\n";
	os << COLOR_LABEL << "Root: " << COLOR_VALUE << req.root << COLOR_RESET << "\n";
	os << COLOR_LABEL << "Server names:" << COLOR_RESET << "\n";

	for (size_t i = 0; i < req.server_name.size(); ++i)
		os << COLOR_VALUE << "  " << req.server_name[i] << "\n";

	os << COLOR_LABEL << "Client max body size: " << COLOR_VALUE << req.client_max_body_size << COLOR_RESET << "\n";
	os << COLOR_LABEL << "Error pages:" << COLOR_RESET << "\n";

	for (std::map<int, std::string>::const_iterator it = req.error_pages.begin(); it != req.error_pages.end(); ++it)
		os << COLOR_VALUE << "  " << it->first << " -> " << it->second << "\n";

	os << COLOR_SECTION << "\nLOCATION:" << COLOR_RESET << "\n";
	os << COLOR_SUBLABEL << "  Location: " << COLOR_VALUE << req.location << COLOR_RESET << "\n";
	os << COLOR_LABEL << "    Methods: " << COLOR_VALUE
	   << "GET[" << req.getAllowed << "] "
	   << "POST[" << req.postAllowed << "] "
	   << "DELETE[" << req.deleteAllowed << "]"
	   << COLOR_RESET << "\n";
	os << COLOR_LABEL << "    Index: " << COLOR_VALUE << req.index << COLOR_RESET << "\n";
	os << COLOR_LABEL << "    Autoindex: " << COLOR_VALUE << req.autoindex << COLOR_RESET << "\n";
	os << COLOR_LABEL << "    Redirect: " << COLOR_VALUE
	   << (req.redirect_enabled ? "enabled" : "disabled")
	   << " -> " << req.redirect_url
	   << " (" << req.redirect_code << ")" << COLOR_RESET << "\n";
	os << COLOR_LABEL << "    Upload store: " << COLOR_VALUE << req.upload_store << COLOR_RESET << "\n";
	os << COLOR_LABEL << "    CGI handlers:" << COLOR_RESET << "\n";

	for (std::map<std::string, std::string>::const_iterator it = req.cgi_pass.begin(); it != req.cgi_pass.end(); ++it)
		os << COLOR_VALUE << "      " << it->first << " -> " << it->second << "\n";

	return os;
}
