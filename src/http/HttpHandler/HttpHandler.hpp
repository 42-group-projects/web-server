#include "../include/enums.hpp"
#include "../../../include/imports.hpp"
#include "../utils.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"
#include "../../fileSystem/FileSystem.hpp"
#include "../../configFileParser/ServerConfig.hpp"

#pragma once

class HttpHandler
{
	private:
		const ServerConfig &config;
		HttpResponse handleGet(const HttpRequest& req);
		// HttpResponse handlePost(const HttpRequest& req);
		// HttpResponse handleDelete(const HttpRequest& req);
		// HttpResponse handleCgi(const HttpRequest& req);
		HttpResponse methodNotAllowed(const HttpRequest& req);

	public:
		// HttpHandler();
		HttpHandler(const ServerConfig &config);
		~HttpHandler();
		HttpResponse handleRequest(const HttpRequest& req);
};