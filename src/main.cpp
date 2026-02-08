#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"
#include "network/NetworkManager.hpp"

int main(int argc, char *argv[])
{
	std::cout << "Web Server Starting..." << std::endl;

	ServerConfig config;
	try {config.initServerConfig(argc, argv);}
		catch (const std::runtime_error& e) {std::cerr << e.what(); return 1;}

	// ネットワークマネージャを起動（最小動作: ヘッダ受信後に固定レスポンスを返す）
	NetworkManager net(config);
	if (!net.init()) {
		std::cerr << "Failed to initialize network listeners. Exiting." << std::endl;
		return 1;
	}
	net.run();
	std::cout << "Web Server Ending..." << std::endl;
	return 0;
}
