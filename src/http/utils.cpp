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

		case PUT:
			return "PUT";

		case PATCH:
			return "PATCH";

		case OPTIONS:
			return "OPTIONS";

		case HEAD:
			return "HEAD";

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
		case RANGE_NOT_SATISFIABLE:
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
	// Debug output to see what we're actually receiving

	if (mimeTypeStr == "text/html")
		return TEXT_HTML;
	else if (mimeTypeStr == "text/css")
		return TEXT_CSS;
	else if (mimeTypeStr == "application/javascript")
		return APPLICATION_JAVASCRIPT;
	else if (mimeTypeStr == "application/json")
		return APPLICATION_JSON;
	else if (mimeTypeStr == "text/plain")
		return TEXT_PLAIN;
	else if (mimeTypeStr == "image/png")
		return IMAGE_PNG;
	else if (mimeTypeStr == "image/jpeg")
		return IMAGE_JPEG;
	else if (mimeTypeStr == "image/gif")
		return IMAGE_GIF;
	else if (mimeTypeStr == "application/pdf")
		return APPLICATION_PDF;
	else if (mimeTypeStr == "image/svg+xml")
		return IMAGE_SVG;
	else if (mimeTypeStr == "application/octet-stream")
		return APPLICATION_OCTET_STREAM;
	else
	{
		std::cout << "DEBUG: No match found, returning NO_FILE" << std::endl;
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

e_status_code getStatusCodeFromInt(int code)
{
	switch (code)
	{
	case 200:
		return OK;
	case 201:
		return CREATED;
	case 202:
		return ACCEPTED;
	case 204:
		return NO_CONTENT;
	case 205:
		return RESET_CONTENT;
	case 206:
		return PARTIAL_CONTENT;
	case 207:
		return MULTI_STATUS;
	case 208:
		return ALREADY_REPORTED;
	case 226:
		return IM_USED;
	case 300:
		return MULTIPLE_CHOICES;
	case 301:
		return MOVED_PERMANENTLY;
	case 302:
		return FOUND;
	case 303:
		return SEE_OTHER;
	case 304:
		return NOT_MODIFIED;
	case 305:
		return USE_PROXY;
	case 307:
		return TEMPORARY_REDIRECT;
	case 308:
		return PERMANENT_REDIRECT;
	case 400:
		return BAD_REQUEST;
	case 401:
		return UNAUTHORIZED;
	case 402:
		return PAYMENT_REQUIRED;
	case 403:
		return FORBIDDEN;
	case 404:
		return NOT_FOUND;
	case 405:
		return METHOD_NOT_ALLOWED;
	case 406:
		return NOT_ACCEPTABLE;
	case 407:
		return PROXY_AUTHENTICATION_REQUIRED;
	case 408:
		return REQUEST_TIMEOUT;
	case 409:
		return CONFLICT;
	case 410:
		return GONE;
	case 411:
		return LENGTH_REQUIRED;
	case 412:
		return PRECONDITION_FAILED;
	case 413:
		return CONTENT_TOO_LARGE;
	case 414:
		return URI_TOO_LONG;
	case 415:
		return UNSUPPORTED_MEDIA_TYPE;
	case 416:
		return RANGE_NOT_SATISFIABLE;
	case 417:
		return EXPECTATION_FAILED;
	case 418:
		return IM_A_TEAPOT;
	case 421:
		return MISDIRECTED_REQUEST;
	case 422:
		return UNPROCESSABLE_ENTITY;
	case 423:
		return LOCKED;
	case 424:
		return FAILED_DEPENDENCY;
	case 425:
		return TOO_EARLY;
	case 426:
		return UPGRADE_REQUIRED;
	case 428:
		return PRECONDITION_REQUIRED;
	case 429:
		return TOO_MANY_REQUESTS;
	case 431:
		return REQUEST_HEADER_FIELDS_TOO_LARGE;
	case 451:
		return UNAVAILABLE_FOR_LEGAL_REASONS;
	case 500:
		return INTERNAL_SERVER_ERROR;
	case 501:
		return NOT_IMPLEMENTED;
	case 502:
		return BAD_GATEWAY;
	case 503:
		return SERVICE_UNAVAILABLE;
	case 504:
		return GATEWAY_TIMEOUT;
	case 505:
		return VERSION_NOT_SUPPORTED;
	case 506:
		return VARIANT_ALSO_NEGOTIATES;
	case 507:
		return INSUFFICIENT_STORAGE;
	case 508:
		return LOOP_DETECTED;
	case 510:
		return NOT_EXTENDED;
	case 511:
		return NETWORK_AUTHENTICATION_REQUIRED;
	default:
		return UNSET;
	}
}

bool hasHost(std::map<std::string, std::string> headers)
{
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
		{

			if (it->first == "Host" || it->first == "host")
				return true;
		}
		return false;
}