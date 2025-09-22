#include "FileInfo.hpp"
#include "SafePath.hpp"

FileInfo::FileInfo(SafePath path) : fullPath(path)
{
    struct stat sb;
    if (stat(fullPath.getFullPath().c_str(), &sb) == 0) {
        isExists = true;
        size = sb.st_size;
        lastModified = sb.st_mtime;
        isDirectory = S_ISDIR(sb.st_mode);
        isReadable = (access(fullPath.getFullPath().c_str(), R_OK) == 0);
    }
    else 
    {
        isExists = false;
        size = 0;
        lastModified = 0;
        isDirectory = false;
        isReadable = false;
    }
}

const SafePath& FileInfo::getPath() const { return fullPath; }
size_t FileInfo::getSize() const { return size; }
time_t FileInfo::getLastModified() const { return lastModified; }
bool FileInfo::exists() const { return isExists; }
bool FileInfo::directory() const { return isDirectory; }
bool FileInfo::readable() const { return isReadable; }