#include "../include/imports.hpp"

#ifndef HTTPRESPONSE_HPP
    #define HTTPRESPONSE_HPP

class   HttpResponse 
{
    public:
        HttpResponse();
        ~HttpResponse();
        void generateResponse(const std::string &status, const std::string &body);
    private:
        std::string status;
        std::string body;
};

#endif
