#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "../include/globals.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "fileSystem/SafePath.hpp"
#include "http/HttpResponse/HttpResponse.hpp"
#include "http/httpTester.hpp"
#include "../src/tests/tests.hpp"
#include "fileSystem/FileSystem.hpp"

ServerConfig g_config;

int main(int argc, char *argv[])
{
	std::cout << "Web Server Starting..." << std::endl;

	try {g_config.initServerConfig(argc, argv);}
		catch (const std::runtime_error& e) {std::cerr << e.what() << std::endl; return 1;}

	//uncommnet to run tests
	http_tester(g_config);
	std::cout << "Web Server Ending..." << std::endl;
	return 0;
}