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
    req.parseRequest(simple_get_request);
    req.displayRequest();
    try
    {
        req.parseRequest(simple_broken_get_request);
    }
    catch (const std::exception &e)
    {
        std::cout << "-----------Error--------------" << std::endl;
        std::cerr << "Error parsing broken request: " << e.what() << std::endl;
        std::cout << std::endl;
    }

    req.parseRequest(simple_post_request);
    req.displayRequest();
    return 0;
}