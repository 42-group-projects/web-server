#pragma once
#include "TokenizeFile.hpp"
#include <string>

namespace error_messages
{
void parserError(const std::string& msg, const t_token& token, const std::string& filePath);
void parserError(const std::string& adj, const std::string& what, const t_token& token, const std::string& filePath);
}
