#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"


#include "http/http_sandbox.hpp"

int main(void)
{
    std::cout << "Web Server Starting..." << std::endl;
    http();
    return 0;
}
