#pragma once
#include "TokenizeFile.hpp"
#include <string>

namespace parser_utils
{
	void expected(const std::string& what, t_token token, const std::string& filePath);
	void unexpected(const std::string& what, t_token token, const std::string& filePath);
	void unclosed(const char what, int line, int col, const std::string& filePath);
}
