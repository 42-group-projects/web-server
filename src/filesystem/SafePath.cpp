#include "../src/filesystem/SafePath.hpp"

SafePath::SafePath(const std::string& path, const ServerConfig& config)
	: requestedPath(path)
{
	if(path.find("..") != std::string::npos)
		throw std::runtime_error("Unsafe path");

	fullPath = config[path].root;

	if(!requestedPath.empty() && requestedPath[0] != '/')
		fullPath += '/';

	fullPath += requestedPath;
}
std::string SafePath::getRequestedPath() const { return requestedPath; }
std::string SafePath::getFullPath() const { return fullPath; }
SafePath::operator std::string() const { return fullPath; }