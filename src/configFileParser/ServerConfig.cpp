#include "ServerConfig.hpp"
#include <iostream>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <fstream>

ServerConfig::ServerConfig(int argc, char **argv)
{
	std::vector<std::string> rawConfig;
	//to do: setup minimal default conf

	if(argc == 1)
	{
		std::cout << "No configuration file provided. Using default configuration file" << std::endl;
		rawConfig = loadConfigFile(DEFAULT_CONF);
	}
	else
	{
		std::string arg(argv[1]);
		size_t ext = arg.rfind(".conf");

		if(ext != std::string::npos && ext + 5 == arg.size())
			rawConfig = loadConfigFile(arg);
		else
		{
			std::cout << "Invalid configuration file. Using default configuration file" << std::endl;
			rawConfig = loadConfigFile(DEFAULT_CONF);
		}
	}

	parseConfig(rawConfig);
}

std::vector<std::string> ServerConfig::loadConfigFile(const std::string& path)
{
	std::ifstream file(path.c_str());

	if(!file)
	{
		if(path == DEFAULT_CONF)
		{
			std::cout << "../default.conf not found. Starting server with built-in default configuration." << std::endl;
			return std::vector<std::string>();
		}
		else
		{
			std::cout << "Couldn't open file. Using default configuration file" << std::endl;
			return loadConfigFile(DEFAULT_CONF);
		}
	}

	std::vector<std::string> lines;
	std::string line;

	while(std::getline(file, line))
		lines.push_back(line);

	return lines;
}

void ServerConfig::parseConfig(std::vector<std::string>& rawConfig)
{
	std::vector<std::string> configVect = normalizeConfig(rawConfig);
	// printVector(configVect); //debug
}

std::vector<std::string>& ServerConfig::normalizeConfig(std::vector<std::string>& rawConfig)
{
	for(int i = rawConfig.size() - 1; i >= 0; i--)
	{
		if(rawConfig[i].empty())
		{
			rawConfig.erase(rawConfig.begin() + i);
			continue;
		}

		removeComments(rawConfig, i);
		trimCollapse(rawConfig, i);
		splitBrackets(rawConfig, i);
	}

	return rawConfig;
}

void ServerConfig::removeComments(std::vector<std::string>& rawConfig, int i)
{
	if(rawConfig[i][0] == '#')
	{
		rawConfig.erase(rawConfig.begin() + i);
		return ;
	}

	size_t pos = rawConfig[i].find('#');

	if(pos != std::string::npos)
		rawConfig[i] = rawConfig[i].substr(0, pos);
}

void ServerConfig::trimCollapse(std::vector<std::string>& rawConfig, int i)
{
	size_t start = 0;
	size_t end = rawConfig[i].size() - 1;

	while(start <= end && (rawConfig[i][start] == ' ' || rawConfig[i][start] == '\t'))
		start++;

	while(end >= start && (rawConfig[i][end] == ' ' || rawConfig[i][end] == '\t'))
		end--;

	if(start > end)
	{
		rawConfig.erase(rawConfig.begin() + i);
		return;
	}

	rawConfig[i] = rawConfig[i].substr(start, end - start + 1);
}

void ServerConfig::splitBrackets(std::vector<std::string>& rawConfig, int i)
{
	int lastCharIndex = rawConfig[i].size() - 1;

	if(!lastCharIndex)
		return;

	char lastChar = rawConfig[i][lastCharIndex];

	if((lastChar == '{' || lastChar == '}'))
	{
		rawConfig[i] = rawConfig[i].substr(0, rawConfig[i].size() - 1);
		rawConfig.insert(rawConfig.begin() + i + 1, std::string(1, lastChar));
	}
}

void ServerConfig::printVector(const std::vector<std::string>& vec) //debug
{
	for(size_t i = 0; i < vec.size(); ++i)
		std::cout << vec[i] << std::endl;
}