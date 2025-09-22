#pragma once

#include <string>
#include <limits>
#include <stdexcept>
#include <unistd.h>

class SafePath
{
private:
    std::string requestedPath;
    std::string fullPath;

public:
    SafePath(const std::string& path);
    std::string getRequestedPath() const;
    std::string getFullPath() const;
    operator std::string() const;
};