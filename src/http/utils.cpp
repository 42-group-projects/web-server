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

e_mimeType getMimeTypeEnum(const std::string mimeTypeStr)
{
	// Trim whitespace and control characters (like \r, \n) from the string
	std::string trimmed = mimeTypeStr;
	
	// Remove trailing whitespace and control characters
	size_t end = trimmed.find_last_not_of(" \t\r\n");
	if (end != std::string::npos) {
		trimmed = trimmed.substr(0, end + 1);
	}
	
	// Remove leading whitespace and control characters
	size_t start = trimmed.find_first_not_of(" \t\r\n");
	if (start != std::string::npos) {
		trimmed = trimmed.substr(start);
	}
	
	std::cout << "DEBUG: Original: '" << mimeTypeStr << "' (length: " << mimeTypeStr.length() << ")" << std::endl;
	std::cout << "DEBUG: Trimmed: '" << trimmed << "' (length: " << trimmed.length() << ")" << std::endl;

	if (trimmed == "text/html")
		return TEXT_HTML;
	else if (trimmed == "text/css")
		return TEXT_CSS;
	else if (trimmed == "application/javascript")
		return APPLICATION_JAVASCRIPT;
	else if (trimmed == "application/json")
		return APPLICATION_JSON;
	else if (trimmed == "text/plain")
		return TEXT_PLAIN;
	else if (trimmed == "image/png")
		return IMAGE_PNG;
	else if (trimmed == "image/jpeg")
		return IMAGE_JPEG;
	else if (trimmed == "image/gif")
		return IMAGE_GIF;
	else if (trimmed == "application/pdf")
		return APPLICATION_PDF;
	else if (trimmed == "image/svg+xml")
		return IMAGE_SVG;
	else if (trimmed == "application/octet-stream")
		return APPLICATION_OCTET_STREAM;
	else
	{
		std::cout << "DEBUG: No match found for trimmed string '" << trimmed << "'" << std::endl;
		return NO_FILE;
	}
}

std::string getMimeTypeExtention(e_mimeType mimeType)
{
	switch (mimeType)
	{
		case TEXT_HTML:
			return ".html";
		case TEXT_CSS:
			return ".css";
		case APPLICATION_JAVASCRIPT:
			return ".js";
		case APPLICATION_JSON:
			return ".json";
		case TEXT_PLAIN:
			return ".txt";
		case IMAGE_PNG:
			return ".png";
		case IMAGE_JPEG:
			return ".jpg";
		case IMAGE_GIF:
			return ".gif";
		case APPLICATION_PDF:
			return ".pdf";
		case IMAGE_SVG:
			return ".svg";
		case APPLICATION_OCTET_STREAM:
			return ".bin";
		default:
			return ".unknown";
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
	std::cout << "MIME Type: " << fs.getMimeType() << std::endl;
	std::cout << "Exists: " << (fs.exists() ? "Yes" : "No") << std::endl;
	std::cout << "Directory: " << (fs.directory() ? "Yes" : "No") << std::endl;
	std::cout << "Readable: " << (fs.readable() ? "Yes" : "No") << std::endl;
	std::cout << "Writable: " << (fs.writable() ? "Yes" : "No") << std::endl;
	std::cout << "Executable: " << (fs.executable() ? "Yes" : "No") << std::endl;
}

void displayConfig(ServerConfig const &config)
{
	std::cout << "-------SERVER CONFIG DETAILS-------" << std::endl;
	std::cout << "Root: " << config.getRoot() << std::endl;
	std::cout << "Server Name: " << config.getServerName() << std::endl;
	std::cout << "Client Max Body Size: " << config.getClientMaxBodySize() << " bytes" << std::endl;

	std::cout << "\nError Pages:" << std::endl;
	std::map<int, std::string> errorPages = config.getErrorPages();
	for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
}

void displayServerConfigDetails(ServerConfig const &config)
{
	std::cout << "-------SERVER CONFIG DETAILS-------" << std::endl;
	std::cout << "Root: " << config.getRoot() << std::endl;
	std::cout << "Server Name: " << config.getServerName() << std::endl;
	std::cout << "Client Max Body Size: " << config.getClientMaxBodySize() << " bytes" << std::endl;

	std::cout << "\nError Pages:" << std::endl;
	std::map<int, std::string> errorPages = config.getErrorPages();
	for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}

	std::cout << "\nListening on:" << std::endl;
	std::vector<std::pair<std::string, int> > listen = config.getListen();
	for (size_t i = 0; i < listen.size(); ++i)
	{
		std::cout << "  " << listen[i].first << ":" << listen[i].second << std::endl;
	}
}

void displayLocationConfigDetails(LocationConfig const &location)
{
	std::cout << "-------LOCATION CONFIG DETAILS-------" << std::endl;
	std::cout << "Location: " << location.location << std::endl;
	std::cout << "Root: " << location.root << std::endl;
	std::cout << "Index: " << location.index << std::endl;
	std::cout << "Autoindex: " << (location.autoindex ? "Enabled" :	 "Disabled") << std::endl;
	std::cout << "GET Allowed: " << (location.getAllowed ? "Yes" : "No") << std::endl;
	std::cout << "POST Allowed: " << (location.postAllowed ? "Yes" : "No") << std::endl;
	std::cout << "DELETE Allowed: " << (location.deleteAllowed ? "Yes" : "No") << std::endl;
	std::cout << "Redirect Enabled: " << (location.redirect_enabled ? "Yes" : "No") << std::endl;
	std::cout << "Redirect Enabled: " << (location.redirect_enabled ? "Yes" : "No") << std::endl;
	std::cout << "Redirect URL: " << location.redirect_url << std::endl;
	std::cout << "Redirect Code: " << location.redirect_code << std::endl;
	std::cout << "Upload Enabled: " << (location.upload_enabled ? "Yes" : "No") << std::endl;
	std::cout << "Upload Store: " << location.upload_store << std::endl;
	std::cout << "CGI Pass:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = location.cgi_pass.begin(); it != location.cgi_pass.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
}
