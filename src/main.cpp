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
#include <iostream>

int main(int argc, char **argv)
{
	TokenizeFile tokenizer(argc, argv);
	const std::vector<t_token>& tokens = tokenizer.getTokens();

	for (size_t i = 0; i < tokens.size(); i++)
		std::cout << "Line " << tokens[i].line + 1
		          << ", Col " << tokens[i].col + 1
		          << ": \"" << tokens[i].str << "\"" << std::endl;

	return 0;
}
