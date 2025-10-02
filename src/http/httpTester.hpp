#include "../../include/imports.hpp"
#include "../../include/enums.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"
#include "../mock_data/http_request.hpp"
#include "../mock_data/http_response.hpp"
#include "./HttpHandler/HttpHandler.hpp"


int http_tester(ServerConfig g_config)
{
	std::cout << "Starting HTTP Tester..." <<std::endl;
	// HttpRequest request(get_request_with_params);
	// request.parseRequest(get_request_with_params);
	// request.displayRequest();
	// HttpResponse response;
	// response.mockData("<response>");
	// response.setHeader("Custom-Header", "CustomValue");
	// response.setHeader("Another-Header", "AnotherValue");
	// response.setMimeType("test/html");
	// response.setVersion("HTTP/1.1");
	// response.setStatus(OK);
	// string res = response.generateResponse(OK, "<html><body><h1>Hello, World!</h1></body></html>");
	// cout << "Generated HTTP Response:\n" << res << endl;

	std::string test_request =
    "YEET /index.html http1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\r\n";

	displayConfigDetails(g_config);

	HttpHandler handler(g_config);
	HttpResponse res = handler.handleRequest(HttpRequest(test_request));
	std::string response = res.generateResponse(res.getStatus());

	std::cout << "Generated HTTP Response:\n";
	std::cout << response << std::endl;
	std::cout << "HTTP Tester Finished." << std::endl;
	return 0;
}

