#include "parser_utils.hpp"
#include "TokenizeFile.hpp"
#include "../errorHandling/ErrorWarning.hpp"

namespace parser_utils
{
	// Error handling
	void expected(const std::string& what, t_token token, const std::string& filePath)
	{
		error("Expected " + what, filePath + ":" + std::to_string(token.line) + ":" + std::to_string(token.col));
	}
	
	void unexpected(const std::string& what, t_token token, const std::string& filePath)
	{
		error("Unexpected " + what, filePath + ":" + std::to_string(token.line) + ":" + std::to_string(token.col));
	}
	
	void unclosed(const char what, int line, int col, const std::string& filePath)
	{
		error("unclosed " + std::string(1, what), filePath + ":" + std::to_string(line) + ":" + std::to_string(col));
	}
}
