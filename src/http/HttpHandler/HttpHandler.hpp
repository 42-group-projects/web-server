#include "../include/enums.hpp"
#include "../../../include/imports.hpp"
#include "../utils.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"
#include "../../fileSystem/FileSystem.hpp"

#pragma once

class HttpHandler
{
	private:
		HttpResponse handleGet(const HttpRequest& req);
		// HttpResponse handlePost(const HttpRequest& req);
		// HttpResponse handleDelete(const HttpRequest& req);

	public:
		HttpHandler();
		~HttpHandler();
		HttpResponse handleRequest(const HttpRequest& req);
};