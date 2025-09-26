#pragma once

#include <iostream>
#include <vector>

#define DEFAULT_CONF "configFiles/default.conf"

class ServerConfig
{
public:
    ServerConfig(int argc, char **argv);
private:

    std::vector<std::string> loadConfigFile(const std::string& path);

    void parseConfig(std::vector<std::string>& rawConfig);

    std::vector<std::string>& normalizeConfig(std::vector<std::string>& rawConfig);
    void removeComments(std::vector<std::string>& rawConfig, int i);
    void trimCollapse(std::vector<std::string>& rawConfig, int i);
    void splitBrackets(std::vector<std::string>& rawConfig, int i);

    void printVector(const std::vector<std::string>& vec);//debug
};