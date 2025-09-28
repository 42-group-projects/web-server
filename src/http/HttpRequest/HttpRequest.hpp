
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
        void parse_request_line(std::istringstream& line_stream);
        void parse_headers(std::istringstream& line_stream);
        void parse_query_params(const std::string& query_string);
    public:
        HttpRequest();
        HttpRequest(const std::string &raw_request);
        HttpRequest &operator=(const HttpRequest &other);
        ~HttpRequest();

        void parseRequest(const std::string &request);
        void displayRequest() const;

        // Getters
        e_method get_method() const { return method; }
        std::string get_uri() const { return uri; }
        std::string get_version() const { return version; }
        std::map<std::string, std::string> get_headers() const { return headers; }
        std::map<std::string, std::string> get_query_params() const { return query_params; }
        std::string get_body() const { return body; }
        // Setters
        void set_method(e_method m) { method = m; }
        void set_uri(const std::string &u) { uri = u; }
        void set_version(const std::string &v) { version = v; }
        void set_body(const std::string &b) { body = b; }
        void set_header(const std::string &key, const std::string &value) { headers[key] = value; }
        void set_query_param(const std::string &key, const std::string &value) { query_params[key] = value; }

        // void parse_body(std::istringstream& line_stream);
};


#endif
