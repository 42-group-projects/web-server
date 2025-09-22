#pragma once

#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include "SafePath.hpp"

class FileInfo
{
private:
    SafePath fullPath;        // "/var/www/html/images/photo.jpg"
    size_t size;                 // 1024768 (bytes)
    time_t lastModified;        // 1640995200 (Unix timestamp)
    bool isExists;                 // true/false
    bool isDirectory;           // true/false 
    bool isReadable;            // true/false

public:
    FileInfo(SafePath path);

    const SafePath& getPath() const;
    size_t getSize() const;
    time_t getLastModified() const;
    bool exists() const;
    bool directory() const;
    bool readable() const;
};