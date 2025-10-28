#include "../../include/imports.hpp"
#include "../../include/enums.hpp"
#include "../http/HttpRequest/HttpRequest.hpp"
#include "../http/HttpResponse/HttpResponse.hpp"
#include "../mock_data/http_request.hpp"
#include "../mock_data/http_response.hpp"
#include "./HttpHandler/HttpHandler.hpp"
#include "../include/globals.hpp"

int http_tester()
{
	std::cout << "Starting HTTP Tester..." <<std::endl;

	std::string test_request =
    "GET /redirect/old.html http1.1\r\n"
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
	"Content-Disposition: form-data; name=\"uploadedfile\"; filename=\"hello.json\" \r\n"
	"\r\n"
	"{\"key1\":\"value1\",\"key2\":2}";

	std::string delete_request =
	"DELETE /upload/test.txt http1.1\r\n"
	"Host: www.example.com\r\n"
	"User-Agent: Mozilla/5.0\r\n"
	"Accept: */*\r\n"
	"\r\n";

	std::string cgi_get_request =
	"GET /cgi/test.sh?param1=value1&param2=value2 HTTP/1.1\r\n"
	"Host: www.example.com\r\n"
	"User-Agent: Mozilla/5.0\r\n"
	"Accept: text/html\r\n"
	"\r\n";

	std::string redirect_get_request =
	"GET /redirect/old.html HTTP/1.1\r\n"
	"Host: www.example.com\r\n"
	"User-Agent: Mozilla/5.0\r\n"
	"Accept: text/html\r\n"
	"\r\n";

	

	// std::cout << "Gconfig: " << g_config << std::endl;

	HttpHandler handler(g_config);

	std::cout << "\n-------HTTP REQUEST-------" << std::endl;
	
	HttpRequest req = HttpRequest(redirect_get_request);
	HttpResponse res = handler.handleRequest(req);
	std::string response = res.generateResponse(res.getStatus());

	std::cout << "\n-------HTTP RESPONSE-------" << std::endl;
	std::cout << "Generated HTTP Response:\n";
	std::cout << response << std::endl;
	std::cout << "HTTP Tester Finished." << std::endl;
	return 0;
}
