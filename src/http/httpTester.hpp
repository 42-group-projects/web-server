#include "../../include/imports.hpp"
#include "../../include/enums.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"
#include "../mock_data/http_request.hpp"
#include "../mock_data/http_response.hpp"
#include "./HttpHandler/HttpHandler.hpp"


namespace std
{
int http_tester(void)
{
	cout << "Starting HTTP Tester..." << endl;
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

	HttpHandler handler;
	// HttpRequest request = HttpRequest(simple_get_request);
	// request.displayRequest();
	HttpResponse res = handler.handleRequest(HttpRequest(simple_get_request));
	cout << "HTTP Tester Finished." << endl;
	return 0;
}
}