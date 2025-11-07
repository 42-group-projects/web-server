#include "./configFileParserV2/ServerConfig.hpp"

#include <iostream>
int main(int argc, char **argv)
{
	try
	{
		ServerConfig configuration;
		configuration.initServerConfig(argc, argv);
		std::cout << configuration << std::endl;
		// keep the exact call form you demanded
		t_request_config conf = configuration.getRequestConfig("localHost", "0.0.0.0", 8080, "/relative-redirect/test");
		// SafePath — use operator std::string or getter
		std::cout << "SafePath.full: " << conf.safePath.getFullPath() << std::endl;
		std::cout << "SafePath.requested: " << conf.safePath.getRequestedPath() << std::endl;
		std::cout << "SafePath.location: " << conf.safePath.getLocation() << std::endl;
		std::cout << "Root: " << conf.root << std::endl;
		std::cout << "Server names:";

		for (size_t i = 0; i < conf.server_name.size(); ++i)
			std::cout << ' ' << conf.server_name[i];

		std::cout << std::endl;
		std::cout << "Location: " << conf.location << std::endl;
		std::cout << "Index: " << conf.index << std::endl;
		std::cout << "Autoindex: " << (conf.autoindex ? "on" : "off") << std::endl;
		std::cout << "Methods allowed: GET=" << (conf.getAllowed ? "1" : "0")
		          << " POST=" << (conf.postAllowed ? "1" : "0")
		          << " DELETE=" << (conf.deleteAllowed ? "1" : "0") << std::endl;
		std::cout << "Client max body size: " << conf.client_max_body_size << std::endl;
		std::cout << "Redirect: enabled=" << (conf.redirect_enabled ? "1" : "0")
		          << " url=" << conf.redirect_url
		          << " code=" << conf.redirect_code << std::endl;
		std::cout << "Upload store: " << conf.upload_store << std::endl;
		std::cout << "CGI passes:" << std::endl;

		for (std::map<std::string, std::string>::const_iterator it = conf.cgi_pass.begin(); it != conf.cgi_pass.end(); ++it)
			std::cout << "  " << it->first << " -> " << it->second << std::endl;

		std::cout << "Error pages:" << std::endl;

		for (std::map<int, std::string>::const_iterator it = conf.error_pages.begin(); it != conf.error_pages.end(); ++it)
			std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}

