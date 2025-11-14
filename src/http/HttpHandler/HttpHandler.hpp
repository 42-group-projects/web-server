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
		t_request_config req_config;
		HttpResponse handleGet(const HttpRequest& req);
		HttpResponse handlePost(const HttpRequest& req);
		HttpResponse handleDelete(const HttpRequest& req);

		//CGI related methods
		bool hasHttpRequestErrors(const HttpRequest& req);
		std::string addAllowHeaders();
		HttpResponse handleErrorPages(const HttpRequest& req, e_status_code response_code);

	public:
		HttpHandler();
		~HttpHandler();
		HttpResponse handleRequest(const HttpRequest& req, const ServerConfig& config);
};
