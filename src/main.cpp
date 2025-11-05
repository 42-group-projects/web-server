// #include "../include/imports.hpp"
// #include "../include/enums.hpp"
#include "../include/globals.hpp"
// #include "http/HttpRequest/HttpRequest.hpp"
// #include "http/HttpResponse/HttpResponse.hpp"
// #include "http/httpTester.hpp"
// #include "network/NetworkManager.hpp"

ServerConfig g_config;

// void errorPagesTests()
// {
// 	FileSystem file(SafePath("/"));
// 	// Here you would perform various checks on file
// 	// to see if we need to respond with an error page
// 	// and if we do ⬇️

// 	file.errorPage(NOT_FOUND); 	// Uses enum e_status_code.
// 								// This will update the metadata of file to match those of
// 								// the error page specified in the config, or a generated
// 								// error page if it is not specified or doesn't exist.

// 	// From here all the usual FileSystem methods can be used to get the metadata and contents
// 	// of the error page.

// 	std::cout << "\033[34mTesting 404 error page defined in the configuration file.\033[0m\n"
// 				 "\033[34mContents should be the whole page with the colors and font set\033[0m" << std::endl << file << std::endl;

// 	file.errorPage(PAYMENT_REQUIRED);

// 	std::cout << "\033[34mTesting 402 error page not defined in the configuration file.\033[0m\n"
// 				 "\033[34mContents should be the minimal generated page\033[0m" << std::endl << file << std::endl;
// }

// int main(int argc, char *argv[])
// {
// 	std::cout << "Web Server Starting..." << std::endl;

// 	try {g_config.initServerConfig(argc, argv);}
// 		catch (const std::runtime_error& e) {std::cerr << e.what(); return 1;}

// 	// errorPagesTests();

// 	// ネットワークマネージャを起動（最小動作: ヘッダ受信後に固定レスポンスを返す）
// 	NetworkManager net;
// 	if (!net.init()) {
// 		std::cerr << "Failed to initialize network listeners. Exiting." << std::endl;
// 		return 1;
// 	}
// 	net.run();
// 	std::cout << "Web Server Ending..." << std::endl;
// 	return 0;
// }

#include "./configFileParserV2/TokenizeFile.hpp"
#include "./configFileParserV2/ServerBlocks.hpp"
#include <iostream>

int main(int argc, char **argv)
{
	try
	{
		TokenizeFile tokenizer(argc, argv);
		const std::vector<t_token>& tokens = tokenizer.getTokens();

		for (size_t i = 0; i < tokens.size(); i++)
			std::cout << "Line " << tokens[i].line
					<< ", Col " << tokens[i].col
					<< ": \"" << tokens[i].str << "\"" << std::endl;

		ServerBlocks serverBlocks(tokenizer);

		const std::vector<t_server_block>& servers = serverBlocks.getServerBlocks();

		for (size_t i = 0; i < servers.size(); i++)
		{
			std::cout << "Server Block " << i + 1 << ":\n";

			for (size_t j = 0; j < servers[i].directives.size(); j++)
			{
				std::cout << "  Directive: " << servers[i].directives[j].directive.str;
				for (size_t k = 0; k < servers[i].directives[j].options.size(); k++)
					std::cout << " " << servers[i].directives[j].options[k].str;
				std::cout << ";\n";
			}

			for (size_t j = 0; j < servers[i].locations.size(); j++)
			{
				std::cout << "  Location " << servers[i].locations[j].name.str << " exact=" << servers[i].locations[j].exact << " {\n";
				for (size_t k = 0; k < servers[i].locations[j].directives.size(); k++)
				{
					std::cout << "    " << servers[i].locations[j].directives[k].directive.str;
					for (size_t l = 0; l < servers[i].locations[j].directives[k].options.size(); l++)
						std::cout << " " << servers[i].locations[j].directives[k].options[l].str;
					std::cout << ";\n";
				}
				std::cout << "  }\n";
			}
			std::cout << std::endl;
		}

	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << '\n';
	}
	return 0;
}
