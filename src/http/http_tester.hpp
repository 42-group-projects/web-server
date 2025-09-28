#include "../../include/imports.hpp"
#include "../../include/enums.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"
#include "../mock_data/http_request.hpp"
#include "../mock_data/http_response.hpp"


namespace std
{

    int http_tester(void)
    {
        cout << "Starting HTTP Tester..." << endl;
        HttpRequest request(get_request_with_params);
        request.parseRequest(get_request_with_params);
        request.displayRequest();
        
        HttpResponse response;
        response.mock_data("<response>");
        response.set_header("Custom-Header", "CustomValue");
        response.set_header("Another-Header", "AnotherValue");
        response.set_mime_type("test/html");
        response.set_version("HTTP/1.1");
        response.set_status(OK);
        
        string res = response.generateResponse(OK , "<html><body><h1>Hello, World!</h1></body></html>");
        cout << "Generated HTTP Response:\n" << res << endl;
        // cout << "HTTP Tester finished." << endl;
        return 0;
    }
}