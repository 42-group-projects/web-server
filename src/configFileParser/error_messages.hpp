#pragma once
#include "TokenizeFile.hpp"
#include <string>

namespace error_messages
{
void expected(const std::string& what, const t_token& token, const std::string& filePath);
void alreadyDefined(const std::string& what, const t_token& token, const std::string& filePath);
void unexpected(const std::string& what, const t_token& token, const std::string& filePath);
void tooBig(const std::string& what, const t_token& token, const std::string& filePath);
void unclosed(const char what, int line, int col, const std::string& filePath);
void missingArgument(const t_token& token, const std::string& filePath);
void missingDirective(const t_token& token, const std::string& what, const std::string& filePath);
void missingLocation(const t_token& token, const std::string& filePath);
void tooManyArguments(const t_token& token, const std::string& filePath);
void notAbsolutePath(const t_token& token, const std::string& filePath);
void pathEndsWithSlash(const t_token& token, const std::string& filePath);
void invalidCharacterInPath(const t_token& token, const std::string& filePath);
void invalidIp(const t_token& token, const std::string& ip, const std::string& filePath);
void invalidPort(const t_token& token, int port, const std::string& filePath);
void invalidPort(const t_token& token, const std::string& portStr, const std::string& filePath);
void notHtml(const t_token& token, const std::string& filePath);
void invalidStatusCode(const t_token& token, const std::string& filePath);
void invalidClientMaxBodySize(const t_token& token, const std::string& value, const std::string& filePath);
void conflictingDirectives(const std::string& other, const t_token& token, const std::string& filePath);
void duplicateDirectives(const std::string& other, const t_token& token, const std::string& filePath);
void unknownArgument(const t_token& token, const std::string& filePath);
void invalidCgiExtension(const t_token& token, const std::string& filePath);
}
