#include "../src/fileSystem/SafePath.hpp"
#include "../include/globals.hpp"


SafePath::SafePath(const std::string& path)
	: requestedPath(path)
{
	if (path.find("..") != std::string::npos)
		throw std::runtime_error("Unsafe path");

	fullPath = g_config[path].root;

	if (!requestedPath.empty() && requestedPath[0] != '/')
		fullPath += '/';

	fullPath += requestedPath;
}
std::string SafePath::getRequestedPath() const { return requestedPath; }
std::string SafePath::getFullPath() const { return fullPath; }
SafePath::operator std::string() const { return fullPath; }