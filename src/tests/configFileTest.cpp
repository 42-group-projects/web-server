#include <vector>
#include <iostream>
#include <map>
#include "../src/configFileParser/ServerConfig.hpp"

void printConfig(const ServerConfig& config)
{
	std::cout << "\033[32mServer configuration (comes from printConfig function, feel free to comment out) " << "\033[0m" << std::endl;
	std::cout << "Server root: " << config[ROOT] << std::endl;
	std::cout << "Server name: " << config[SERVER_NAME] << std::endl;
	std::cout << "Client max body size: " << config[CLIENT_MAX_BODY_SIZE] << std::endl;
	std::cout << "Listen:" << std::endl;
	const std::vector<std::pair<std::string, int> >& listen = config[LISTEN];

	for (std::vector<std::pair<std::string, int> >::const_iterator it = listen.begin(); it != listen.end(); ++it)
		std::cout << "  " << it->first << ":" << it->second << std::endl;

	std::cout << "Error pages:" << std::endl;
	const std::map<int, std::string>& error_pages = config[ERROR_PAGES];

	for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
		std::cout << "  " << it->first << " -> " << it->second << std::endl;

	std::cout << std::endl;
	std::cout << "Locations:" << std::endl;
	const std::map<std::string, LocationConfig>& locations = config[LOCATIONS];

	for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		const LocationConfig& loc = it->second;
		std::cout << "  Path: " << loc.path << std::endl;
		std::cout << "    Root: " << loc.root << std::endl;
		std::cout << "    Index: " << loc.index << std::endl;
		std::cout << "    Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
		std::cout << "    GET: " << (loc.getAllowed ? "yes" : "no")
		          << ", POST: " << (loc.postAllowed ? "yes" : "no")
		          << ", DELETE: " << (loc.deleteAllowed ? "yes" : "no") << std::endl;
		std::cout << "    Redirect: " << (loc.redirect_enabled ? loc.redirect_url : "none") << std::endl;
		std::cout << "    Redirect: " << loc.redirect_code << std::endl;
		std::cout << "    Upload: " << (loc.upload_enabled ? loc.upload_store : "disabled") << std::endl;

		if (!loc.cgi_pass.empty())
		{
			std::cout << "    CGI:" << std::endl;

			for (std::map<std::string, std::string>::const_iterator cit = loc.cgi_pass.begin(); cit != loc.cgi_pass.end(); ++cit)
				std::cout << "      " << cit->first << " -> " << cit->second << std::endl;
		}

		std::cout << std::endl;
	}

	std::cout << "Accessing a location by name" << std::endl;
	const LocationConfig& loc = config["/redirection/old.html"];
	std::cout << "Path: " << loc.path << std::endl;
	std::cout << "  Root: " << loc.root << std::endl;
	std::cout << "  Index: " << loc.index << std::endl;
	std::cout << "  Autoindex: " << (loc.autoindex ? "on" : "off") << std::endl;
	std::cout << "  GET: " << (loc.getAllowed ? "yes" : "no")
	          << ", POST: " << (loc.postAllowed ? "yes" : "no")
	          << ", DELETE: " << (loc.deleteAllowed ? "yes" : "no") << std::endl;
	std::cout << "  Redirect: " << (loc.redirect_enabled ? loc.redirect_url : "none") << std::endl;
	std::cout << "  Redirect code: " << loc.redirect_code << std::endl;
	std::cout << "  Upload: " << (loc.upload_enabled ? loc.upload_store : "disabled") << std::endl;

	if (!loc.cgi_pass.empty())
	{
		std::cout << "  CGI:" << std::endl;

		for (std::map<std::string, std::string>::const_iterator cit = loc.cgi_pass.begin(); cit != loc.cgi_pass.end(); ++cit)
			std::cout << "    " << cit->first << " -> " << cit->second << std::endl;
	}

	std::cout << "Accessing a non existent location by name (should generate default configs)" << std::endl;
	const LocationConfig& loc2 = config["/test"];
	std::cout << "Path: " << loc2.path << std::endl;
	std::cout << "  Root: " << loc2.root << std::endl;
	std::cout << "  Index: " << loc2.index << std::endl;
	std::cout << "  Autoindex: " << (loc2.autoindex ? "on" : "off") << std::endl;
	std::cout << "  GET: " << (loc2.getAllowed ? "yes" : "no")
	          << ", POST: " << (loc2.postAllowed ? "yes" : "no")
	          << ", DELETE: " << (loc2.deleteAllowed ? "yes" : "no") << std::endl;
	std::cout << "  Redirect: " << (loc2.redirect_enabled ? loc2.redirect_url : "none") << std::endl;
	std::cout << "  Redirect code: " << loc2.redirect_code << std::endl;
	std::cout << "  Upload: " << (loc2.upload_enabled ? loc2.upload_store : "disabled") << std::endl;

	if (!loc2.cgi_pass.empty())
	{
		std::cout << "  CGI:" << std::endl;

		for (std::map<std::string, std::string>::const_iterator cit = loc2.cgi_pass.begin(); cit != loc2.cgi_pass.end(); ++cit)
			std::cout << "    " << cit->first << " -> " << cit->second << std::endl;
	}

	std::cout << std::endl;
}
