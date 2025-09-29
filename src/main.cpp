#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "../include/globals.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"

#include "http/httpTester.hpp"
#include "../src/tests/tests.hpp"

ServerConfig g_config;

int main(int argc, char *argv[])
{
	g_config.initServerConfig(argc, argv);
	printConfig(g_config);
	std::cout << "Web Server Starting..." << std::endl;
	//uncommnet to run tests
	// std::http_tester();
	std::cout << "Web Server Ending..." << std::endl;
	return 0;
}