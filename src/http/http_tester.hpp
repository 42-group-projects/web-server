#include "../../include/imports.hpp"
#include "../../include/enums.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"
#include "../mock_data/http_request.hpp"
#include "../mock_data/http_response.hpp"

int http_tester(void)
{
    HttpRequest  req;
    HttpResponse res;

    std::cout << "----- HTTP REQUEST TESTER -----" << std::endl;
    req.parseRequest(simple_get_request);
    std::cout << "-------------------------------" << std::endl;
    req.displayRequest();
    std::cout << "-------------------------------" << std::endl;

    try
    {
        std::cout << "----- HTTP REQUEST TESTER -----" << std::endl;
        req.parseRequest(simple_missing_request_line_request);
        std::cout << "-------------------------------" << std::endl;
        req.displayRequest();
        std::cout << "-------------------------------" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error parsing broken request: " << e.what() << std::endl;
    }

    // req.parseRequest(simple_post_request);
    // // req.displayRequest();
    // req.parseRequest(get_request_with_params);
    // req.displayRequest();


    return 0;
}