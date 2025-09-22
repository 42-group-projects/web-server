#include "../include/imports.hpp"
#include "../include/enums.hpp"

#ifndef HTTPRESPONSE_HPP
    #define HTTPRESPONSE_HPP

class   HttpResponse 
{
    public:
        HttpResponse();
        ~HttpResponse();
        void generateResponse(const std::string &status, const std::string &body);
    private:
        e_status_code status;
        std::string body;
};

#endif
