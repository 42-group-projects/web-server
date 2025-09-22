
#include "../include/imports.hpp"
#include "../include/enums.hpp"


#ifndef HTTPREQUEST_HPP
    #define HTTPREQUEST_HPP

class   HttpRequest 
{
    public:
        HttpRequest();
        ~HttpRequest();
        void parseRequest(const std::string &request);
        void displayRequest() const;
    private:
        e_method method;
        std::string uri;
        std::string version;
        // Add other necessary members;
};

#endif
