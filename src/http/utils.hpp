#pragma once
#include "enums.hpp"
#include "../../include/imports.hpp"
#include "./HttpRequest/HttpRequest.hpp"
#include "./HttpResponse/HttpResponse.hpp"
#include "./../fileSystem/FileSystem.hpp"
#include "./../configFileParser/ServerConfig.hpp"

std::string getMethodString(e_method method);
std::string getMimeTypeString(e_mimeType mimeType);
e_mimeType getMimeTypeEnum(const std::string mimeTypeStr);
std::string getMimeTypeExtention(e_mimeType mimeType);
std::string getStatusString(e_status_code status);
std::string getCurrentTime();
e_status_code getStatusCodeFromInt(int code);
bool hasHost(std::map<std::string, std::string> headers);
