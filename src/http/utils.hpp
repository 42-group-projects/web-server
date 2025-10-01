#pragma once
#include "enums.hpp"
#include "../../include/imports.hpp"
#include "./HttpRequest/HttpRequest.hpp"
#include "./HttpResponse/HttpResponse.hpp"
#include "./../fileSystem/FileSystem.hpp"
#include <string>

std::string get_method_string(e_method method);
std::string get_current_time();
void displayFileSystemInfo(FileSystem const &fs);
