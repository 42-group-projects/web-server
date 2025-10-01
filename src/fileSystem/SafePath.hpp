#pragma once

#include <iostream>
#include <stdexcept>
#include <vector>

class SafePath
{
private:
	std::string requestedPath;
	std::string fullPath;
	std::string location;

public:
	SafePath(const std::string& path);

	std::vector<std::string> splitPath(const std::string &path);
	std::string getRequestedPath() const;
	std::string getFullPath() const;
	std::string getLocation() const;
	operator std::string() const;
};

std::ostream& operator<<(std::ostream& os, const SafePath& sp);
