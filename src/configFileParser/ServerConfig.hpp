#pragma once

#include <iostream>
#include <vector>

class ServerConfig
{
public:
    ServerConfig(int argc, char **argv);
private:

    std::vector<std::string> loadConfigFile(const std::string& path);
    std::vector<std::string> loadStringConfig(const std::string& string);
    void parseConfig(std::vector<std::string>& rawConfig);
    void ServerConfig::normalizeConfig(std::vector<std::string>& rawConfig);
};