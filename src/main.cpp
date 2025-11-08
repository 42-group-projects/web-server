#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "../include/globals.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"
#include "http/httpTester.hpp"
#include "network/NetworkManager.hpp"

ServerConfig g_config;

int main(int argc, char *argv[])
{
	std::cout << "Web Server Starting..." << std::endl;

	try {g_config.initServerConfig(argc, argv);}
		catch (const std::runtime_error& e) {std::cerr << e.what(); return 1;}
	// std::cout << g_config << std::endl;
	// errorPagesTests();

	// ネットワークマネージャを起動（最小動作: ヘッダ受信後に固定レスポンスを返す）
	NetworkManager net;
	if (!net.init()) {
		std::cerr << "Failed to initialize network listeners. Exiting." << std::endl;
		return 1;
	}
	net.run();
	std::cout << "Web Server Ending..." << std::endl;
	return 0;
}
