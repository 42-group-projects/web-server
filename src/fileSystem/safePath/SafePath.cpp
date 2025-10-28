#include "../src/fileSystem/safePath/SafePath.hpp"
#include "../include/globals.hpp"
#include "../src/errorHandling/ErrorWarning.hpp"


SafePath::SafePath(const std::string& path)
	: requestedPath(path)
{
	if (path[0] != '/')
		error("Invalid path '" + requestedPath + "' must start with /", "SafePath");

	if (path.find("..") != std::string::npos)
		error("Unsafe path '" + requestedPath + "'", "SafePath");

	std::vector<std::string> splited = splitPath(path);
	location = "/";
	size_t longest = 0;

	for (std::map<std::string, LocationConfig>::const_iterator it = g_config.getLocations().begin();
	        it != g_config.getLocations().end(); ++it)
	{
		const std::string& loc = it->first;

		if (requestedPath.compare(0, loc.size(), loc) == 0 && loc.size() > longest)
		{
			location = loc;
			longest = loc.size();
		}
	}

	std::string root = g_config[location].root;

	if (location == "/")
		fullPath = root + requestedPath;
	else
	{
		std::string remainder = requestedPath.substr(location.size());
		fullPath = root + remainder;
	}
}

std::vector<std::string> SafePath::splitPath(const std::string &path)
{
	std::vector<std::string> parts;
	std::string current;

	for (size_t i = 0; i < path.size(); ++i)
	{
		if (path[i] == '/')
		{
			if (!current.empty())
			{
				parts.push_back(current);
				current.clear();
			}
		}
		else
			current += path[i];
	}

	if (!current.empty())
		parts.push_back(current);

	return parts;
}

std::string SafePath::getRequestedPath() const { return requestedPath; }
std::string SafePath::getFullPath() const { return fullPath; }
std::string SafePath::getLocation() const { return location; }
SafePath::operator std::string() const { return fullPath; }

std::ostream& operator<<(std::ostream& os, const SafePath& sp) { return os << std::string(sp); }