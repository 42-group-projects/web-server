#include <iostream>
#include "SafePath.hpp"
#include "FileInfo.hpp"

void FileInfo_SafePath_tests() {
    try {
        SafePath sp("www/index.html");
        FileInfo info(sp);

        std::cout << "Full path: " << info.getPath().getFullPath() << std::endl;
        std::cout << "Requested path: " << info.getPath().getRequestedPath() << std::endl;
        std::cout << "Exists: " << (info.exists() ? "yes" : "no") << std::endl;
        std::cout << "Readable: " << (info.readable() ? "yes" : "no") << std::endl;
        std::cout << "Directory: " << (info.directory() ? "yes" : "no") << std::endl;
        std::cout << "Size: " << info.getSize() << " bytes" << std::endl;
        std::cout << "Last modified: " << info.getLastModified() << " (unix time)" << std::endl;

        try {
            SafePath badPath("../etc/passwd");
            FileInfo badInfo(badPath);
        } catch (const std::exception& e) {
            std::cout << "Blocked unsafe path: " << e.what() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
