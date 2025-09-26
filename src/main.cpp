#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"

#include "../src/configFileParser/ServerConfig.hpp"

#include "http/http_tester.hpp"
#include "../src/tests/tests.hpp"

int main(int argc, char *argv[])
{
	ServerConfig config(argc, argv);

	std::cout << "Web Server Starting..." << std::endl;
	http_tester();
	std::cout << "Web Server Ending..." << std::endl;
	return 0;
}
