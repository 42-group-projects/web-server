#include "Signals.hpp"
#include <iostream>

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
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}
