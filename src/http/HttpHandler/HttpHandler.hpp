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

		bool hasHttpRequestErrors(const HttpRequest& req);
		std::string addAllowHeaders();
		HttpResponse handleErrorPages(const HttpRequest& req, e_status_code response_code);
		HttpResponse handleRedirects(const HttpRequest& req);
		bool isCgiRequest(const HttpRequest& req);
		void finalizeResponse(HttpResponse& res, const HttpRequest& req);

		HttpResponse writeFile(const HttpRequest& req, const std::string& file_name, const std::string& content);

	public:
		HttpHandler();
		~HttpHandler();

		// HttpResponse handleRequest(const HttpRequest& req, const ServerConfig& config);
		//==============================================================================================
		HttpResponse handleRequest(const HttpRequest& req, const ServerConfig& config, std::string& ip, int port);
		//=======================================================================================Clement
};
