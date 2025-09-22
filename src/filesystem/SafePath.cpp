#include "SafePath.hpp"

SafePath::SafePath(const std::string& path) : requestedPath(path)
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) 
        throw std::runtime_error("Failed to get working directory");

    std::string workingDir(cwd);
    std::string absolutePath = workingDir + "/" + requestedPath;

    char resolvedPath[PATH_MAX];
    if (!realpath(absolutePath.c_str(), resolvedPath))
        throw std::runtime_error("Failed to resolve path");

    fullPath = std::string(resolvedPath);

    if (fullPath.compare(0, workingDir.length(), workingDir) != 0)
        throw std::runtime_error("Path escapes server root");
}

std::string SafePath::getFullPath() const { return fullPath; }
std::string SafePath::getRequestedPath() const { return requestedPath; }
SafePath::operator std::string() const { return fullPath; }