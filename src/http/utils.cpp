#include "enums.hpp"
#include "../../include/imports.hpp"

std::string get_method_string(e_method method)
{
	switch(method)
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
