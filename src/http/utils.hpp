#pragma once
#include "enums.hpp"
#include "../../include/imports.hpp"
#include "./HttpRequest/HttpRequest.hpp"
#include "./HttpResponse/HttpResponse.hpp"
#include "./../fileSystem/FileSystem.hpp"
#include <string>

std::string getMethodString(e_method method);
std::string getMimeTypeString(e_mimeType mimeType);
std::string getStatusString(e_status_code status);
std::string getCurrentTime();
void displayFileSystemInfo(FileSystem const &fs);
