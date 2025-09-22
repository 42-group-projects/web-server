#include "../include/imports.hpp"
#include "../include/enums.hpp"
#include "http/HttpRequest/HttpRequest.hpp"
#include "http/HttpResponse/HttpResponse.hpp"

#include "tests.hpp"

int main(void)
{
    // Tests
    FileInfo_SafePath_tests();
    //------------------------

    std::cout << "Web Server Starting..." << std::endl;
    return 0;
}
