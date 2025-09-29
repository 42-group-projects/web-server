#include "enums.hpp"
#include "../../include/imports.hpp"

std::string get_method_string(e_method method)
{
	switch (method)
	{
	case GET:
		return "GET";

	case POST:
		return "POST";

	case DELETE:
		return "DELETE";

	default:
		return "UNKNOWN";
	}
}

std:: string get_current_time()
{
	time_t now = time(NULL);
	struct tm *tm_info = gmtime(&now);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
	return std::string(buffer);
}



