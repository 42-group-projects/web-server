#include "../include/imports.hpp"

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