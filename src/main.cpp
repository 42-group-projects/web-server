#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"

#include "../src/configFileParser/ServerConfig.hpp"

#include "http/httpTester.hpp"
#include "../src/tests/tests.hpp"

int main(int argc, char *argv[])
{
	ServerConfig config(argc, argv);

	std::cout << "Web Server Starting..." << std::endl;
	//uncommnet to run tests
	// std::http_tester();
	std::cout << "Web Server Ending..." << std::endl;
	return 0;
}