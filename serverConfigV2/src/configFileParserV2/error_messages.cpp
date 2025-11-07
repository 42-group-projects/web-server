#include "error_messages.hpp"
#include "TokenizeFile.hpp"
#include "../../../src/errorHandling/ErrorWarning.hpp"
#include <sstream>

namespace error_messages
{
static std::string toStr(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

void expected(const std::string& what, const t_token& token, const std::string& filePath)
{
	error("Expected " + what,
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void unexpected(const std::string& what, const t_token& token, const std::string& filePath)
{
	error("Unexpected " + what,
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void unclosed(const char what, int line, int col, const std::string& filePath)
{
	error("Unclosed " + std::string(1, what),
	      filePath + ":" + toStr(line) + ":" + toStr(col));
}

void missingArgument(const t_token& token, const std::string& filePath)
{
	error("Missing argument for directive " + token.str,
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void missingDirective(const t_token& token, const std::string& what, const std::string& filePath)
{
	error("Missing " + what + " directive.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void missingLocation(const t_token& token, const std::string& filePath)
{
	error("Missing '/' location.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void tooManyArguments(const t_token& token, const std::string& filePath)
{
	error("Too many arguments for directive " + token.str,
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void notAbsolutePath(const t_token& token, const std::string& filePath)
{
	error("\"" + token.str + "\" is not an absolute path.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void pathEndsWithSlash(const t_token& token, const std::string& filePath)
{
	error("\"" + token.str + "\" cannot end with '/'.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void invalidIp(const t_token& token, const std::string& ip, const std::string& filePath)
{
	error(ip + " is not a valid IP.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void invalidPort(const t_token& token, int port, const std::string& filePath)
{
	error(toStr(port) + " is not a valid port.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void invalidPort(const t_token& token, const std::string& portStr, const std::string& filePath)
{
	error(portStr + " is not a valid port.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void notHtml(const t_token& token, const std::string& filePath)
{
	error(token.str + " must be of type html.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void invalidStatusCode(const t_token& token, const std::string& filePath)
{
	error(token.str + " is not a valid status code.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void invalidClientMaxBodySize(const t_token& token, const std::string& value, const std::string& filePath)
{
	error(value + " is not a valid client_max_body_size value.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void unknownDirective(const t_token& token, const std::string& filePath)
{
	error("Unknown directive \"" + token.str + "\".",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void unknownArgument(const t_token& token, const std::string& filePath)
{
	error("Unknown argument \"" + token.str + "\".",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void invalidCgiExtension(const t_token& token, const std::string& filePath)
{
	error(token.str + " is not a valid cgi extension.",
	      filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}
}
