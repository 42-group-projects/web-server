#include <iostream>
#include <vector>
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

void testDetectMimeType() {
    std::vector<std::string> paths;
    paths.push_back("mimeTypeDetectionTestFiles/index.html");
    paths.push_back("mimeTypeDetectionTestFiles/style.CSS");
    paths.push_back("mimeTypeDetectionTestFiles/script.Js");
    paths.push_back("mimeTypeDetectionTestFiles/data.JSON");
    paths.push_back("mimeTypeDetectionTestFiles/readme.txt");
    paths.push_back("mimeTypeDetectionTestFiles/image.PNG");
    paths.push_back("mimeTypeDetectionTestFiles/photo.jpeg");
    paths.push_back("mimeTypeDetectionTestFiles/graphic.GIF");
    paths.push_back("mimeTypeDetectionTestFiles/document.pdf");
    paths.push_back("mimeTypeDetectionTestFiles/vector.SvG");
    paths.push_back("mimeTypeDetectionTestFiles/unknownfile.abc");
    paths.push_back("mimeTypeDetectionTestFiles/noextension");
    paths.push_back("mimeTypeDetectionTestFiles/nonexistent.file");

    for (std::vector<std::string>::const_iterator it = paths.begin(); it != paths.end(); ++it) {
        const std::string& p = *it;
        SafePath sp(p);
        FileInfo file(sp);
        e_mimeType type = file.getMimeType();
        std::cout << "Path: " << p << " -> MIME: " << mimeTypeToString(type) << std::endl;
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
        std::cout << "Writable: " << (info.writable() ? "yes" : "no") << std::endl;
        std::cout << "Executable: " << (info.executable() ? "yes" : "no") << std::endl;
        std::cout << "Directory: " << (info.directory() ? "yes" : "no") << std::endl;
        std::cout << "Size: " << info.getSize() << " bytes" << std::endl;
        std::cout << "Last modified: " << info.getLastModified() << " (unix time)" << std::endl;
        std::cout << "Mime type: " << mimeTypeToString(info.getMimeType()) << std::endl;

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    std::cout << std::endl;
}
