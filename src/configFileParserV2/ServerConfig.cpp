#include "ServerConfig.hpp"

ServerConfig::ServerConfig(int argc, char **argv)
{
	TokenizeFile tokenizer(argc, argv);
	ServerBlocks serverBlocks(tokenizer);
	const std::vector<t_server_block> &servers = serverBlocks.getServerBlocks();

	for (size_t i = 0; i < servers.size(); i++)
		configuration.push_back(setServerConfig(servers[i]));
}

t_server_config ServerConfig::setServerConfig(const t_server_block &serverBlock)
{
	t_server_config config;
	config.root = "";
	config.listen.clear();
	config.server_name.push_back(SERVER_NAME);
	config.error_pages.clear();
	config.client_max_body_size = CLIENT_MAX_BODY_SIZE;
	
	for (size_t i = 0; i < serverBlock.directives.size(); i++)
	{
		t_directive directive = serverBlock.directives[i];
		if (directive.directive.str == "root")
			setRoot(directive.options);
		else if (directive.directive.str == "listen")
			setListen(directive.options);
		else if (directive.directive.str == "server_name")
			setServerName(directive.options);
		else if (directive.directive.str == "error_page")
			setErrorPage(directive.options);
		else if (directive.directive.str == "client_max_body_size")
			setClientMaxBodySize(directive.options);
		else if (directive.directive.str == "location")
			setLocation(directive.options, configVect, &i);
		else
			directiveError(directive.directive.str);
	}
	// checkConfig();
	return config;
}

void ServerConfig::setRoot(std::vector<std::string>& tokens)
{
	if (tokens.size() == 1)
		missingArgument(tokens[0]);

	if (tokens.size() > 2)
		argumentError(tokens[2], tokens[0]);

	if (tokens[1][0] != '/')
		error(tokens[1] + " is not an absolute path.", "Configuration file");

	if (tokens[1][tokens[1].size() - 1] == '/')
		error("Invalid root argument: '" + tokens[1] + "' should not end with '/'", "Configuration file.");

	root = tokens[1];
}