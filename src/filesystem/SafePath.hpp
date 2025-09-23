#pragma once

#include <string>
#include <stdexcept>

class SafePath
{
private:
	std::string requestedPath;
	std::string fullPath;
	const std::string serverRoot;

public:
	SafePath(const std::string& path);

	std::string getRequestedPath() const;
	std::string getFullPath() const;
	operator std::string() const;
};
