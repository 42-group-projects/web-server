#include <iostream>
#include "SafePath.hpp"
#include "FileInfo.hpp"

std::string mimeTypeToString(e_mimeType type) {
    switch (type) {
        case NO_FILE: return "";
        case TEXT_HTML: return "text/html";
        case TEXT_CSS: return "text/css";
        case APPLICATION_JAVASCRIPT: return "application/javascript";
        case APPLICATION_JSON: return "application/json";
        case TEXT_PLAIN: return "text/plain";
        case IMAGE_PNG: return "image/png";
        case IMAGE_JPEG: return "image/jpeg";
        case IMAGE_GIF: return "image/gif";
        case APPLICATION_PDF: return "application/pdf";
        case IMAGE_SVG: return "image/svg+xml";
        case APPLICATION_OCTET_STREAM: return "application/octet-stream";
        default: return "application/octet-stream";
    }
}

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
        std::cout << "Mime type: " << mimeTypeToString(info.getMimeType()) << std::endl;

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::cout << std::endl;
}
