#pragma once

#include <string>
#include <stdexcept>

#include "../src/configFileParser/ServerConfig.hpp"

class SafePath
{
private:
	std::string requestedPath;
	std::string fullPath;

public:
	SafePath(const std::string& path);

	std::string getRequestedPath() const;
	std::string getFullPath() const;
	operator std::string() const;
};
