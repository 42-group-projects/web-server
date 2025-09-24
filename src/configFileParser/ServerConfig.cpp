#include "ServerConfig.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <fstream>

ServerConfig::ServerConfig(int argc, char **argv)
{
    if (argc == 1)
        throw std::runtime_error("No configuration file. Usage: ./webser <config file or string>");
    
    std::string arg(argv[1]);
    size_t ext = arg.rfind(".conf");

    std::vector<std::string> rawConfig;
    if (ext != std::string::npos && ext + 5 == arg.size())
    {
        rawConfig = loadConfigFile(arg);
    }
    else
    {
        rawConfig = loadStringConfig(arg);
    }

    parseConfig(rawConfig);
}

std::vector<std::string> ServerConfig::loadConfigFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
    {
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> ServerConfig::loadStringConfig(const std::string& string)
{
    std::vector<std::string> lines;
    std::istringstream ss(string);
    std::string line;

    while (std::getline(ss, line))
    {
        lines.push_back(line);
    }
    return lines;
}

void ServerConfig::parseConfig(std::vector<std::string>& rawConfig)
{
    normalizeConfig(rawConfig);
}

void ServerConfig::normalizeConfig(std::vector<std::string>& rawConfig)
{
    for (size_t i = 0; i < rawConfig.size(); ++i)
    {
        std::cout << rawConfig[i] << "\n";
        rawConfig.erase(rawConfig.begin() + i)
    }
}
