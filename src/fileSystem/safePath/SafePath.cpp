#include "SafePath.hpp"
#include "../src/errorHandling/ErrorWarning.hpp"
#include "../../configFileParser/ServerConfig.hpp"

struct t_location_config;
struct t_server_config;

SafePath::SafePath() : requestedPath(""), fullPath(""), location("") {}

SafePath::SafePath(const std::string& path, const t_server_config* serverConf)
	: requestedPath(path)
{
	if (path.find("..") != std::string::npos)
		error("Unsafe path '" + requestedPath + "'", "SafePath");

	std::vector<std::string> reqSegments = splitPath(path);
	location = "/";
	size_t longest = 0;

	for (std::map<std::string, t_location_config>::const_iterator it = serverConf->locations.begin();
	        it != serverConf->locations.end(); ++it)
	{
		const std::string& loc = it->first;
		const t_location_config& conf = it->second;
		std::vector<std::string> locSegments = splitPath(loc);

		bool match = false;

		if (conf.exact)
		{
			match = (reqSegments == locSegments);
		}
		else
		{
			if (reqSegments.size() >= locSegments.size())
			{
				match = true;
				for (size_t i = 0; i < locSegments.size(); ++i)
				{
					if (reqSegments[i] != locSegments[i])
					{
						match = false;
						break;
					}
				}
			}
		}

		if (match && loc.size() > longest)
		{
			location = loc;
			longest = loc.size();
		}
	}

	std::string root = serverConf->locations.at(location).root;

	if (location == "/")
		fullPath = root + requestedPath;
	else
	{
		std::vector<std::string> locSegments = splitPath(location);
		std::string remainder;
		for (size_t i = locSegments.size(); i < reqSegments.size(); ++i)
			remainder += "/" + reqSegments[i];
		fullPath = root + remainder;
	}
}


SafePath::SafePath(const std::string& requestedPath, const t_request_config& req_conf)
{
	if (requestedPath.find("..") != std::string::npos)
		error("Unsafe path '" + requestedPath + "'", "SafePath");

	location = req_conf.location;

	std::string root = req_conf.root;

	if (location == "/")
		fullPath = root + requestedPath;
	else
	{
		std::string remainder = requestedPath.substr(location.size());
		fullPath = root + remainder;
	}
	std::cout << "Full path resolved: " << fullPath << std::endl;
}


SafePath& SafePath::operator=(const SafePath& other)
{
	if (this != &other)
	{
		this->requestedPath = other.requestedPath;
		this->fullPath = other.fullPath;
		this->location = other.location;
	}
	return *this;
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

void SafePath::setFullPath(const std::string& path)	{ fullPath = path; }

std::string SafePath::getRequestedPath() const { return requestedPath; }
std::string SafePath::getFullPath() const { return fullPath; }
std::string SafePath::getLocation() const { return location; }
SafePath::operator std::string() const { return fullPath; }

std::ostream& operator<<(std::ostream& os, const SafePath& sp) { return os << std::string(sp); }