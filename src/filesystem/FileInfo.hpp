#pragma once

#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>
#include "SafePath.hpp"
#include "enums.hpp"

class FileInfo
{
private:
    SafePath fullPath;        // "/var/www/html/images/photo.jpg"
    size_t size;                 // 1024768 (bytes)
    time_t lastModified;        // 1640995200 (Unix timestamp)
    e_mimeType mimeType;
    bool isExists;                 // true/false
    bool isDirectory;           // true/false 
    bool isReadable;            // true/false
    bool isWritable;            // true/false
    bool isExecutable;            // true/false

    e_mimeType detectMimeType(const SafePath& safePath);

public:
    FileInfo(SafePath path);

    const SafePath& getPath() const;
    size_t getSize() const;
    time_t getLastModified() const;
    e_mimeType getMimeType() const;
    bool exists() const;
    bool directory() const;
    bool readable() const;
    bool writable() const;
    bool executable() const;
};