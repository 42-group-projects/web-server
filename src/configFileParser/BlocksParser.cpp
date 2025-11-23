#include "BlocksParser.hpp"
#include "error_messages.hpp"
#include "arg_validity_checks.hpp"
#include "../../src/errorHandling/ErrorWarning.hpp"

#include <sstream>
#include <cstdlib>

BlocksParser::BlocksParser
(
	const std::vector<t_server_block> &servers,
	std::string &f,
	std::vector<t_server_config>& c,
	std::vector<std::pair<std::string, int> >& a
) : filePath(f), configuration(c), allListen(a)
{
	defaultServerSet = false;
	for (size_t i = 0; i < servers.size(); i++)
		configuration.push_back(setServerConfig(servers[i]));

	if (hasDuplicateServerBlocks())
		error("Error: duplicate server_name:listen combination detected", "Configuration file");
}

t_server_config BlocksParser::setServerConfig(const t_server_block &serverBlock)
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

std::string BlocksParser::setRoot(const t_directive& directive)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);
	arg_validity_checks::checkPathEndsWithSlash(directive.options[0], filePath);
	arg_validity_checks::checkSpecialCharacters(directive.options[0], filePath);
	return directive.options[0].str;
}

std::pair<std::string, int> BlocksParser::setListen(const t_directive& directive)
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

std::vector<std::string> BlocksParser::setServerName(const t_directive& directive)
{
	arg_validity_checks::optionsCount(directive, 1, filePath);
	std::vector<std::string> res;

	for (size_t i = 0; i < directive.options.size(); i++)
		res.push_back(directive.options[i].str);

	return res;
}

void BlocksParser::setErrorPage(const t_directive& directive, std::map<int, std::string>& errorPages)
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
size_t BlocksParser::setClientMaxBodySize(const t_directive& directive)
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

void BlocksParser::setLocation(const t_location_block& locBlock, std::map<std::string, t_location_config>& locations, const std::string& root)
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

t_location_config BlocksParser::setupDefaultLocationConfig(const std::string& location, const std::string& root)
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

void BlocksParser::setMethods(const t_directive& directive, t_location_config& conf)
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

void BlocksParser::setIndex(const t_directive& directive, t_location_config& conf)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);
	conf.index = directive.options[0].str;
}

void BlocksParser::setOnOffDirective(const t_directive& directive, bool& field)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);

	if (directive.options[0].str == "on")
		field = true;
	else if (directive.options[0].str == "off")
		field = false;
	else
		error_messages::unknownArgument(directive.options[0], filePath);
}

void BlocksParser::setUploadPath(const t_directive& directive, t_location_config& conf)
{
	arg_validity_checks::optionsCount(directive, 1, 1, filePath);
	conf.upload_store = directive.options[0].str;
}

void BlocksParser::setRedirect(const t_directive& directive, t_location_config& conf)
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

void BlocksParser::setCgi(const t_directive& directive, t_location_config& conf)
{
	arg_validity_checks::optionsCount(directive, 1, 2, filePath);

	if (!(directive.options[0].str.size() > 1 && directive.options[0].str[0] == '.'))
		error_messages::invalidCgiExtension(directive.options[0], filePath);

	conf.cgi_pass[directive.options[0].str] = directive.options[1].str;
}

bool BlocksParser::hasDuplicateServerBlocks() const
{
	std::set<std::string> seen;

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
					return true;

				seen.insert(key.str());
			}
		}
	}

	return false;
}
