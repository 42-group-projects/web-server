#include "error_messages.hpp"
#include "TokenizeFile.hpp"
#include "../../src/errorHandling/ErrorWarning.hpp"
#include <sstream>

namespace error_messages
{
static std::string toStr(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

static std::string origin(const t_token& token, const std::string& filePath)
{
	return std::string(filePath + ":" + toStr(token.line) + ":" + toStr(token.col));
}

void parserError(const std::string& msg, const t_token& token, const std::string& filePath)
{
	error(msg + ".", origin(token, filePath));
}

void parserError(const std::string& adj, const std::string& what, const t_token& token, const std::string& filePath)
{
	error(adj + " " + what + ".", origin(token, filePath));
}
}
