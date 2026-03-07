#include "Signals.hpp"
#include "../../include/imports.hpp"

static NetworkManager *g_net = NULL;

static void signalHandler(int signum)
{
	std::cerr << "\nSignal " << signum << " received, shutting down gracefully..." << std::endl;
	if (g_net)
		g_net->stop();
}

void setSignalNetworkManager(NetworkManager *net)
{
	g_net = net;
}

void setupSignalHandlers()
{
	signal(SIGINT,  signalHandler);
	signal(SIGTERM, signalHandler);
}
