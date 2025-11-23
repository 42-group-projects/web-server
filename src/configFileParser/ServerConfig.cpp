#include "ServerConfig.hpp"
#include "error_messages.hpp"
#include "arg_validity_checks.hpp"
#include "../../src/errorHandling/ErrorWarning.hpp"

#include <sstream>
#include <cstdlib>

ServerConfig::ServerConfig() {}

void ServerConfig::initServerConfig(int argc, char **argv)
{
	TokenizeFile tokenizer(argc, argv);
	filePath = tokenizer.getFilePath();
	ServerBlocks serverBlocks(tokenizer);
	const std::vector<t_server_block> &servers = serverBlocks.getServerBlocks();
	BlocksParser(servers, filePath, configuration, allListen);
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
