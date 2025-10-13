#pragma once

#include "enums.hpp"
#include <string>

class ErrorPageGenerator
{
private:
	std::string html;
	
	const char* statusCodeToString(int code);

public:
	ErrorPageGenerator(e_status_code code);
	const std::string getHtml() const;
};


