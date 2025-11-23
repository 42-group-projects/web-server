#include "arg_validity_checks.hpp"
#include "../errorHandling/ErrorWarning.hpp"

#include <sstream>

namespace arg_validity_checks
{
void optionsCount(const t_directive& directive, unsigned int min, std::string& p)
{
	if (directive.options.size() < min)
		error_messages::parserError("Missing argument for directive", directive.directive.str, directive.directive, p);
}

void optionsCount(const t_directive& directive, unsigned int min, unsigned int max, std::string& p)
{
	optionsCount(directive, min, p);

	if (directive.options.size() > max)
		error_messages::parserError("Too many arguments for directive", directive.directive.str, directive.directive, p);
}

void checkAbsolutePath(const t_token& token, std::string& p)
{
	if (token.str[0] != '/')
		error_messages::parserError(token.str, "is not an absolute path", token, p);
}

void checkPathEndsWithSlash(const t_token& token, std::string& p)
{
	if (token.str[token.str.size() - 1] == '/')
		error_messages::parserError(token.str, "cannot end with '/'", token, p);
}

void checkIP(const t_token& token, std::string& ip, std::string& p)
{
	std::stringstream ss(ip);
	std::string segment;
	int count = 0;

	while (std::getline(ss, segment, '.'))
	{
		if (segment.empty() || segment.size() > 3)
			error_messages::parserError(token.str, "is nto a valid IP adress", token, p);

		for (std::string::size_type i = 0; i < segment.size(); ++i)
			if (!std::isdigit(segment[i]))
				error_messages::parserError(token.str, "is nto a valid IP adress", token, p);

		int num = std::atoi(segment.c_str());

		if (num < 0 || num > 255)
			error_messages::parserError(token.str, "is nto a valid IP adress", token, p);

		count++;
	}

	if (count != 4)
		error_messages::parserError(token.str, "is nto a valid IP adress", token, p);
}

void checkPort(const t_token& token, int port, std::string& p)
{
	if (port < 1 || port > 65535)
		error_messages::parserError(token.str, "is nto a valid port", token, p);
}

void isValidStatusCode(const t_directive& directive, std::string& p)
{
	static const int codes[] =
	{
		200, 201, 202, 204, 205, 206, 207, 208, 226,
		300, 301, 302, 303, 304, 305, 306, 307, 308,
		400, 401, 402, 403, 404, 405, 406, 407, 408, 409,
		410, 411, 412, 413, 414, 415, 416, 417, 418, 421,
		422, 423, 424, 425, 426, 428, 429, 431, 451,
		500, 501, 502, 503, 504, 505, 506, 507, 508, 510, 511
	};

	for (size_t i = 0; i + 1 < directive.options.size(); ++i)
	{
		const std::string& s = directive.options[i].str;

		if (!isNumber(s))
			error_messages::parserError(directive.options[i].str, "is not a valid status code", directive.options[i], p);

		int code = std::atoi(s.c_str());
		bool valid = false;

		for (size_t j = 0; j < sizeof(codes) / sizeof(codes[0]); ++j)
		{
			if (code == codes[j])
			{
				valid = true;
				break;
			}
		}

		if (!valid)
			error_messages::parserError(directive.options[i].str, "is not a valid status code", directive.options[i], p);
	}
}

bool isNumber(const std::string &s)
{
	if (s.empty())
		return false;

	for (std::string::size_type i = 0; i < s.size(); ++i)
		if (!std::isdigit(s[i]))
			return false;

	return true;
}
}
