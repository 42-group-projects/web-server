#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "../include/globals.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"
#include "http/httpTester.hpp"

#include "directoryListing/DirectoryListing.hpp" // for test

ServerConfig g_config;

int main(int argc, char *argv[])
{
	std::cout << "Web Server Starting..." << std::endl;

	try {g_config.initServerConfig(argc, argv);}
		catch (const std::runtime_error& e) {std::cerr << e.what(); return 1;}

	//directory listing test
	//{
		SafePath sp("/mimeType");
		DirectoryListing list(sp);
		std::cout << std::endl << list.getHtml() << std::endl;
	//}
	
	//uncommnet to run tests
	// http_tester(g_config);
	std::cout << "Web Server Ending..." << std::endl;
	return 0;
}
