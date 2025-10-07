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

	std::string test_request =
    "GET /index.html http1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\r\n";

	std::string bad_test_request =
    "SET /index.html http1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Accept: text/html\r\n"
    "\r\n";

	std::string post_request_with_json =
	"POST /upload http1.1\r\n"
	"Host: www.example.com\r\n"
	"User-Agent: Mozilla/5.0\r\n"
	"Content-Type: application/json\r\n"
	"Content-Length: 32\r\n"
	"\r\n"
	"{\"key1\":\"value1\",\"key2\":2}";

	HttpHandler handler(g_config);
	HttpRequest req(bad_test_request);
	req.displayRequest();

	HttpResponse res = handler.handleRequest(HttpRequest(bad_test_request));
	std::string response = res.generateResponse(res.getStatus());
	std::cout << "\n-------HTTP RESPONSE-------" << std::endl;
	std::cout << "Generated HTTP Response:\n";
	std::cout << response << std::endl;
	std::cout << "HTTP Tester Finished." << std::endl;
	return 0;
}
