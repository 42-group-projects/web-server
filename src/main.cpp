#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"


#include "http/http_tester.hpp"

#include "tests.hpp"

int main(void)
{
    // Tests
    FileInfo_SafePath_tests("test");
    FileInfo_SafePath_tests("errors/400.html");
    FileInfo_SafePath_tests("/");
    FileInfo_SafePath_tests("/../..");

    testDetectMimeType();
    //------------------------ This is meant for whoever will review my PR. Feel free to delete when it's merged - Clement

    std::cout << "Web Server Starting..." << std::endl;
    http_tester();
    std::cout << "Web Server Ending..." << std::endl;
    return 0;
}
