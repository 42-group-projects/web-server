
#include "../include/imports.hpp"

#ifndef HTTPREQUEST_HPP
    #define HTTPREQUEST_HPP

class   HttpRequest 
{
    public:
        HttpRequest();
        ~HttpRequest();
        void parseRequest(const std::string &request);
    private:
        std::string method;
        std::string uri;
        std::string version;
};

#endif