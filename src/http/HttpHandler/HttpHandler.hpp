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
		HttpResponse handlePost(const HttpRequest& req);
		HttpResponse handleDelete(const HttpRequest& req);
		// TODO: implement POST AND DELETE AND CGI
		// HttpResponse handleDelete(const HttpRequest& req);
		// HttpResponse handleCgi(const HttpRequest& req);

		HttpResponse handleErrorPages(const HttpRequest& req, e_status_code code);

	public:
		HttpHandler(const ServerConfig &config);
		~HttpHandler();
		HttpResponse handleRequest(const HttpRequest& req);
};
