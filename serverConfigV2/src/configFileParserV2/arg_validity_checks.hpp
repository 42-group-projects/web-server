#pragma once
#include "ServerConfig.hpp"
#include "error_messages.hpp"

#include <string>

namespace arg_validity_checks
{
void optionsCount(const t_directive& directive, unsigned int min, std::string& p);
void optionsCount(const t_directive& directive, unsigned int min, unsigned int max, std::string& p);
void checkAbsolutePath(const t_token& token, std::string& p);
void checkPathEndsWithSlash(const t_token& token, std::string& p);
void checkIP(const t_token& token, std::string& ip, std::string& p);
void checkPort(const t_token& token, int port, std::string& p);
void isValidStatusCode(const t_directive& directive, std::string& p);
bool isNumber(const std::string &s);
void endsWithHtml(const t_token& token, std::string& p);
}