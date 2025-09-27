#include "../src/filesystem/SafePath.hpp"

SafePath::SafePath(const std::string& path) : requestedPath(path), serverRoot("./www")
{
	if(path.find("..") != std::string::npos)
		throw std::runtime_error("Path escapes server root");

	fullPath = serverRoot;

	if(!requestedPath.empty() && requestedPath[0] != '/')
		fullPath += '/';

	fullPath += requestedPath;
}

std::string SafePath::getRequestedPath() const { return requestedPath; }
std::string SafePath::getFullPath() const { return fullPath; }
SafePath::operator std::string() const { return fullPath; }