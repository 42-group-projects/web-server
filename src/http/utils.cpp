#include "enums.hpp"
#include "../../include/imports.hpp"
#include "./utils.hpp"

std::string getMethodString(e_method method)
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

std::string getStatusString(e_status_code status)
{
	switch (status)
	{
		case UNSET:
			return "Unset";
		case OK:
			return "OK";
		case CREATED:
			return "Created";
		case ACCEPTED:
			return "Accepted";
		case NO_CONTENT:
			return "No Content";
		case RESET_CONTENT:
			return "Reset Content";
		case PARTIAL_CONTENT:
			return "Partial Content";
		case MULTI_STATUS:
			return "Multi-Status";
		case ALREADY_REPORTED:
			return "Already Reported";
		case IM_USED:
			return "IM Used";
		case MULTIPLE_CHOICES:
			return "Multiple Choices";
		case MOVED_PERMANENTLY:
			return "Moved Permanently";
		case FOUND:
			return "Found";
		case SEE_OTHER:
			return "See Other";
		case NOT_MODIFIED:
			return "Not Modified";
		case USE_PROXY:
			return "Use Proxy";
		case UNUSED:
			return "Unused";
		case TEMPORARY_REDIRECT:
			return "Temporary Redirect";
		case PERMANENT_REDIRECT:
			return "Permanent Redirect";
		case BAD_REQUEST:
			return "Bad Request";
		case UNAUTHORIZED:
			return "Unauthorized";
		case PAYMENT_REQUIRED:
			return "Payment Required";
		case FORBIDDEN:
			return "Forbidden";
		case NOT_FOUND:
			return "Not Found";
		case METHOD_NOT_ALLOWED:
			return "Method Not Allowed";
		case NOT_ACCEPTABLE:
			return "Not Acceptable";
		case PROXY_AUTHENTICATION_REQUIRED:
			return "Proxy Authentication Required";
		case REQUEST_TIMEOUT:
			return "Request Timeout";
		case CONFLICT:
			return "Conflict";
		case GONE:
			return "Gone";
		case LENGTH_REQUIRED:
			return "Length Required";
		case PRECONDITION_FAILED:
			return "Precondition Failed";
		case CONTENT_TOO_LARGE:
			return "Content Too Large";
		case URI_TOO_LONG:
			return "URI Too Long";
		case UNSUPPORTED_MEDIA_TYPE:
			return "Unsupported Media Type";
		case RANGE_NOT_SATISFIABLEwebserver:
			return "Range Not Satisfiable";
		case EXPECTATION_FAILED:
			return "Expectation Failed";
		case IM_A_TEAPOT:
			return "I'm a Teapot";
		case MISDIRECTED_REQUEST:
			return "Misdirected Request";
		case UNPROCESSABLE_ENTITY:
			return "Unprocessable Entity";
		case LOCKED:
			return "Locked";
		case FAILED_DEPENDENCY:
			return "Failed Dependency";
		case TOO_EARLY:
			return "Too Early";
		case UPGRADE_REQUIRED:
			return "Upgrade Required";
		case PRECONDITION_REQUIRED:
			return "Precondition Required";
		case TOO_MANY_REQUESTS:
			return "Too Many Requests";
		case REQUEST_HEADER_FIELDS_TOO_LARGE:
			return "Request Header Fields Too Large";
		case UNAVAILABLE_FOR_LEGAL_REASONS:
			return "Unavailable For Legal Reasons";
		case INTERNAL_SERVER_ERROR:
			return "Internal Server Error";
		case NOT_IMPLEMENTED:
			return "Not Implemented";
		case BAD_GATEWAY:
			return "Bad Gateway";
		case SERVICE_UNAVAILABLE:
			return "Service Unavailable";
		case GATEWAY_TIMEOUT:
			return "Gateway Timeout";
		case VERSION_NOT_SUPPORTED:
			return "HTTP Version Not Supported";
		case VARIANT_ALSO_NEGOTIATES:
			return "Variant Also Negotiates";
		case INSUFFICIENT_STORAGE:
			return "Insufficient Storage";
		case LOOP_DETECTED:
			return "Loop Detected";
		case NOT_EXTENDED:
			return "Not Extended";
		case NETWORK_AUTHENTICATION_REQUIRED:
			return "Network Authentication Required";
		default:
			return "Unknown Status Code";
	}
}

std::string getMimeTypeString(e_mimeType mimeType)
{
	switch (mimeType)
	{
		case NO_FILE:
			return "no_file";

		case TEXT_HTML:
			return "text/html";

		case TEXT_CSS:
			return "text/css";

		case APPLICATION_JAVASCRIPT:
			return "application/javascript";

		case APPLICATION_JSON:
			return "application/json";

		case TEXT_PLAIN:
			return "text/plain";

		case IMAGE_PNG:
			return "image/png";

		case IMAGE_JPEG:
			return "image/jpeg";

		case IMAGE_GIF:
			return "image/gif";

		case APPLICATION_PDF:
			return "application/pdf";

		case IMAGE_SVG:
			return "image/svg+xml";

		case APPLICATION_OCTET_STREAM:
			return "application/octet-stream";

		default:
			return "unknown";
	}
}

std:: string getCurrentTime()
{
	time_t now = time(NULL);
	struct tm *tm_info = gmtime(&now);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
	return std::string(buffer);
}

void displayFileSystemInfo(FileSystem const &fs)
{
	std::cout << "-------FILESYSTEM INFO-------" << std::endl;
	std::cout << "Path: " << fs.getPath().getFullPath() << std::endl;
	std::cout << "Size: " << fs.getSize() << " bytes" << std::endl;
	// std::cout << "Last Modified: " << ctime(&fs.getLastModified());
	std::cout << "MIME Type: " << fs.getMimeType() << std::endl;
	std::cout << "Exists: " << (fs.exists() ? "Yes" : "No") << std::endl;
	std::cout << "Directory: " << (fs.directory() ? "Yes" : "No") << std::endl;
	std::cout << "Readable: " << (fs.readable() ? "Yes" : "No") << std::endl;
	std::cout << "Writable: " << (fs.writable() ? "Yes" : "No") << std::endl;
	std::cout << "Executable: " << (fs.executable() ? "Yes" : "No") << std::endl;
}


