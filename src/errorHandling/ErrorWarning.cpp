#include "ErrorWarning.hpp"

void error(const std::string& msg, const std::string& origin)
{
	std::cerr << "\033[1m\033[31mError: \033[0m\033[1m" << origin << "\033[0m: " << msg << std::endl;
	std::exit(1);
}

void warning(const std::string& msg, const std::string& origin)
{
	std::cerr << "\033[1m\033[33mWarning: \033[0m\033[1m" << origin << "\033[0m: " << msg << std::endl;
}
