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
		HttpResponse handleGet(const HttpRequest& req);
		HttpResponse handlePost(const HttpRequest& req);
		HttpResponse handleDelete(const HttpRequest& req);

		bool hasHttpRequestErrors(const HttpRequest& req);
		std::string addAllowHeaders();
		HttpResponse handleRedirects(const HttpRequest& req);
		
		HttpResponse writeFile(const HttpRequest& req, const std::string& file_name, const std::string& content);
		
		public:
		t_request_config req_config;
		HttpHandler();
		~HttpHandler();
		
		HttpResponse handleErrorPages(const HttpRequest& req, e_status_code response_code);
		HttpResponse handleRequest(const HttpRequest& req, const ServerConfig& config, std::string& ip, int port);
		bool isCgiRequest(const HttpRequest& req);
		bool loadRequestConfig(const HttpRequest& req, const ServerConfig& config, const std::string& ip, int port);
};
