#include "../../../include/imports.hpp"
#include "../include/enums.hpp"
#include "../../errorHandling/ErrorWarning.hpp"

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

class   HttpRequest
{
	private:
		e_method method;
		std::string uri;
		std::string version;
		std::map<std::string, std::string> headers;
		std::map<std::string, std::string> query_params;
		std::string quary_string;
		std::string body;
		std::string parsing_error;

		// Helper parsing functions
		void parseRequestLine(std::istringstream& line_stream);
		void parseHeaders(std::istringstream& line_stream);
		void parseQueryParams(const std::string& query_string);

	public:
		// Constructors, destructor, and assignment operator
		HttpRequest();
		HttpRequest(const std::string &raw_request);
		HttpRequest &operator=(const HttpRequest &other);
		~HttpRequest();

		// Main parsing function
		void parseRequest(const std::string &request);
		void displayRequest() const;

		// Getters
		e_method getMethod() const { return method; }
		std::string getUri() const { return uri; }
		std::string getVersion() const { return version; }
		std::map<std::string, std::string> getHeaders() const { return headers; }
		std::map<std::string, std::string> getQueryParams() const { return query_params; }
		std::string getQueryString() const { return quary_string; }
		std::string getBody() const { return body; }
		std::string getParsingError() const { return parsing_error; }
		std::string getMimeTypeString() const;
		// Setters
		void setMethod(e_method m) { method = m; }
		void setUri(const std::string &u) { uri = u; }
		void setVersion(const std::string &v) { version = v; }
		void setHeader(const std::string &key, const std::string &value) { headers[key] = value; }
		void setQueryParam(const std::string &key, const std::string &value) { query_params[key] = value; }
		void setQuaryString(const std::string &qs) { quary_string = qs; }
		void setBody(const std::string &b) { body = b; }
		void setParsingError(std::string message ) { parsing_error = message; }
};

#endif
