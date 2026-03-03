#pragma once

#include <signal.h>
#include "../network/NetworkManager.hpp"

// Set the NetworkManager instance that will be stopped on signal.
void setSignalNetworkManager(NetworkManager *net);

// Register SIGINT and SIGTERM handlers.
void setupSignalHandlers();
