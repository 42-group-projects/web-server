#pragma once

#include "../src/fileSystem/SafePath.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <string>

struct FileEntry
{
    std::string name;
    bool isDir;
};

class DirectoryListing {
private:
    SafePath path;
    std::string html;
    static const std::string liStart;
    static const std::string hrefEnd;
    static const std::string liEnd;
    static const std::string htmlTemplate;

    std::vector<FileEntry> listDir(const std::string& path);
    std::string createLinkList(std::vector<FileEntry> entries);

public:
    DirectoryListing(SafePath path);
    const std::string& getHtml() const;
};


