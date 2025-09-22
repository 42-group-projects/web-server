#include <iostream>
#include "SafePath.hpp"
#include "FileInfo.hpp"

void FileInfo_SafePath_tests(std::string path) {
    try {
        SafePath sp(path);
        std::cout << "Safe path OK: " << sp.getFullPath() << std::endl;
        std::cout << "Requested path: " << sp.getRequestedPath() << std::endl;

        FileInfo info(sp);
        std::cout << "Exists: " << (info.exists() ? "yes" : "no") << std::endl;
        std::cout << "Readable: " << (info.readable() ? "yes" : "no") << std::endl;
        std::cout << "Directory: " << (info.directory() ? "yes" : "no") << std::endl;
        std::cout << "Size: " << info.getSize() << " bytes" << std::endl;
        std::cout << "Last modified: " << info.getLastModified() << " (unix time)" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::cout << std::endl;
}
