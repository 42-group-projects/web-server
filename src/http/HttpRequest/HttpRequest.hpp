
#include "../../../include/imports.hpp"
#include "../include/enums.hpp"

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
        std::string body;
        void parseRequestLine(std::istringstream& line_stream);
        void parseHeaders(std::istringstream& line_stream);
        void parseQueryParams(const std::string& query_string);
    public:
        HttpRequest();
        HttpRequest(const std::string &raw_request);
        HttpRequest &operator=(const HttpRequest &other);
        ~HttpRequest();

        void parseRequest(const std::string &request);
        void displayRequest() const;

        // Getters
        e_method getMethod() const { return method; }
        std::string getUri() const { return uri; }
        std::string getVersion() const { return version; }
        std::map<std::string, std::string> getHeaders() const { return headers; }
        std::map<std::string, std::string> getQueryParams() const { return query_params; }
        std::string getBody() const { return body; }
        // Setters
        void setMethod(e_method m) { method = m; }
        void setUri(const std::string &u) { uri = u; }
        void setVersion(const std::string &v) { version = v; }
        void setBody(const std::string &b) { body = b; }
        void setHeader(const std::string &key, const std::string &value) { headers[key] = value; }
        void setQueryParam(const std::string &key, const std::string &value) { query_params[key] = value; }

};


#endif
