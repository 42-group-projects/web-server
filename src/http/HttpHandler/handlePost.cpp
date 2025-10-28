#include "HttpHandler.hpp"
#include "../utils.hpp"
#include <fstream>
#include <unistd.h>

std::string formatFileName(const HttpRequest &req);
std::string getFileNamFromHeader(const HttpRequest &req);

HttpResponse HttpHandler::handlePost(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());
	FileSystem fs(SafePath(req.getUri()));
	LocationConfig location_config = config[fs];
	// int code = OK; ( not used )

	if (g_config[fs.getPath()].redirect_enabled)
	{
		// code = g_config[fs.getPath()].redirect_code;
		fs = FileSystem(SafePath(g_config[fs.getPath()].redirect_url));
	}

	//error checking and validations
	if(location_config.postAllowed == false)
		return handleErrorPages(req, FORBIDDEN);
	if(req.getBody().size() > config.getClientMaxBodySize())
		return handleErrorPages(req, CONTENT_TOO_LARGE);


	if(location_config.upload_enabled)
	{
		if(location_config.upload_store.empty())
		{
			// TODO: or makde uplaod store mandatory if upload is enabled
			error("Upload store not specified in location block", "HttpHandler::handlePost");
		}

		std::string upload_path = fs.getPath();
		std::string file_name = formatFileName(req);

		try
		{
			std::stringstream file_path_ss;
			file_path_ss << location_config.root << "/" << file_name;

			// Ensure the there are no duplicate file names
			FileSystem check_file(SafePath(req.getUri() + "/" + file_name));
			if (check_file.exists())
				error("File already exists: " + file_name, "HttpHandler::handlePost");

			std::ofstream outfile(file_path_ss.str().c_str(), std::ios::binary);
			if (!outfile.is_open())
			{
				error("Failed to open file for writing", "HttpHandler::handlePost");
			}
			else
			{
				outfile << req.getBody();
				outfile.close();

				// do i need to over ride this status code if there is a redirection?
				res.setStatus(CREATED);
				res.setMimeType("text/plain");
				res.setHeader("Location", location_config.upload_store + "/" + file_name);
				res.setBody("File uploaded successfully as " + file_name + "\n");
				return res;
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return handleErrorPages(req, INTERNAL_SERVER_ERROR);
		}
	}
	return res;
}

std::string formatFileName(const HttpRequest &req)
{
	std::string file_name;

	try
	{
		file_name  = getFileNamFromHeader(req);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	if(file_name.empty())
	{
		// TODO: generate a unique filename not using a timestamp
		// to avoid potential collisions in high-frequency uploads
		std::stringstream ss;
		e_mimeType mime_enum = getMimeTypeEnum(req.getMimeTypeString());
		std::string ext = getMimeTypeExtention(mime_enum);
		ss << "upload_file_" << time(NULL) << ext;
		file_name = ss.str();
	}

	return file_name;
}

std::string getFileNamFromHeader(const HttpRequest &req)
{
	std::string file_name;
	std::map<std::string, std::string> headers = req.getHeaders();
	std::map<std::string, std::string>::const_iterator it = headers.find("Content-Disposition");

	if(it != headers.end())
	{
		std::string value = it->second;
		value.erase(0, value.find_first_not_of(" \"\t\r\n"));
		value.erase(value.find_last_not_of(" \"\t\r\n") + 1);

		size_t filename_pos = value.find("filename=");
		if (filename_pos != std::string::npos)
		{
			// 9 is the length of "filename="
			// 1 is for the opening quote
			size_t start = filename_pos + 9 + 1;
			size_t end = value.find_first_of(";\r\n", start);
			if (end == std::string::npos)
				end = value.length();
			file_name = value.substr(start, end - start);
		}
		else
		{
			error("Filename not found in Content-Disposition header", "HttpHandler::getFileNamFromHeader");
		}
	}
	return file_name;
}	