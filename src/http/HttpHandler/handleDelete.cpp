#include "HttpHandler.hpp"
#include "../utils.hpp"



HttpResponse HttpHandler::handleDelete(const HttpRequest& req)
{
	return handleErrorPages(req, NOT_IMPLEMENTED);
}