#include "HttpHandler.hpp"
#include "../utils.hpp"
#include <fstream>
#include <unistd.h>


struct MultipartFile {
	std::string filename;
	std::string content;
	std::string contentType;
};

std::vector<MultipartFile> parseMultipartFormData(const std::string& body, const std::string& boundary);
std::string extractBoundary(const std::string& contentType);
std::string formatFileName(const HttpRequest &req);
std::string getFileNamFromHeader(const HttpRequest &req);

HttpResponse HttpHandler::handlePost(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());
	FileSystem fs(req_config.safePath, req_config);

	if(!req_config.postAllowed && req_config.upload_store.empty())
		return handleErrorPages(req, METHOD_NOT_ALLOWED);

	if(req.getBody().size() > req_config.client_max_body_size)
		return handleErrorPages(req, CONTENT_TOO_LARGE);

	std::string contentType = req.getMimeTypeString();

	// Check if this is multipart/form-data
	if (contentType.find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = extractBoundary(contentType);
		if (boundary.empty())
			return handleErrorPages(req, BAD_REQUEST);

		std::vector<MultipartFile> files = parseMultipartFormData(req.getBody(), boundary);
		if (files.empty())
			return handleErrorPages(req, BAD_REQUEST);

		std::stringstream uploaded_files;
		for (size_t i = 0; i < files.size(); i++)
		{
			HttpResponse fileRes = writeFile(req, files[i].filename, files[i].content);
			if (fileRes.getStatus() != CREATED)
				return fileRes; // Return error if any file fails
			uploaded_files << files[i].filename;
			if (i < files.size() - 1)
				uploaded_files << ", ";
		}

		res.setStatus(CREATED);
		res.setMimeType("text/plain");
		res.setBody("Files uploaded successfully: " + uploaded_files.str() + "\n");
		return res;
	}

	// single file upload
	std::string file_name = formatFileName(req);
	return writeFile(req, file_name, req.getBody());
}

HttpResponse HttpHandler::writeFile(const HttpRequest& req, const std::string& file_name, const std::string& content)
{
	HttpResponse res;
	res.setVersion(req.getVersion());

	try
	{
		std::stringstream file_path_ss;
		file_path_ss << req_config.upload_store << "/" << file_name;

		FileSystem check_file(SafePath(file_path_ss.str(), req_config), req_config);
		if (check_file.exists())
		{
			warning("File already exists: " + file_name, "HttpHandler::writeFile");
			return HttpHandler::handleErrorPages(req, CONFLICT);
		}

		std::ofstream outfile(file_path_ss.str().c_str(), std::ios::binary);
		if (!outfile.is_open())
		{
			warning("Failed to open file for writing", "HttpHandler::writeFile");
			return HttpHandler::handleErrorPages(req, INTERNAL_SERVER_ERROR);
		}

		outfile << content;
		outfile.close();

		res.setStatus(CREATED);
		res.setMimeType("text/plain");
		res.setHeader("Location", req_config.upload_store + "/" + file_name);
		res.setBody("File uploaded successfully as " + file_name + "\n");
		return res;
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return HttpHandler::handleErrorPages(req, INTERNAL_SERVER_ERROR);
	}
}

std::string extractBoundary(const std::string& contentType)
{
	size_t pos = contentType.find("boundary=");
	if (pos == std::string::npos)
		return "";

	std::string boundary = contentType.substr(pos + 9);
	boundary.erase(0, boundary.find_first_not_of(" \t\""));
	boundary.erase(boundary.find_last_not_of(" \t\"\r\n") + 1);
	return "--" + boundary;
}

std::vector<MultipartFile> parseMultipartFormData(const std::string& body, const std::string& boundary)
{
	std::vector<MultipartFile> files;
	size_t pos = 0;

	while ((pos = body.find(boundary, pos)) != std::string::npos)
	{
		pos += boundary.length();
		if (body.substr(pos, 2) == "--")
			break; // End boundary

		size_t header_end = body.find("\r\n\r\n", pos);
		if (header_end == std::string::npos)
			break;

		std::string headers = body.substr(pos, header_end - pos);
		size_t content_start = header_end + 4;
		size_t content_end = body.find(boundary, content_start);
		if (content_end == std::string::npos)
			break;

		// Extract filename
		size_t filename_pos = headers.find("filename=\"");
		if (filename_pos != std::string::npos)
		{
			MultipartFile file;
			size_t name_start = filename_pos + 10;
			size_t name_end = headers.find("\"", name_start);
			file.filename = headers.substr(name_start, name_end - name_start);
			file.content = body.substr(content_start, content_end - content_start - 2); // -2 for \r\n
			files.push_back(file);
		}

		pos = content_end;
	}

	return files;
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

	// might have to delelte this block later if we decide to make filename mandatory
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

		if (value.empty())
		{
			error("Empty Content-Disposition header value", "HttpHandler::getFileNamFromHeader");
			return file_name;
		}

		size_t filename_pos = value.find("filename=");
		if (filename_pos != std::string::npos)
		{
			// 9 is the length of "filename="
			// 1 is for the opening quote
			size_t start = filename_pos + 9 + 1;

			std::cout << "Content-Disposition header value: " << value << std::endl;
			if (start >= value.length())
			{
				error("Malformed Content-Disposition header: filename value too short", "HttpHandler::getFileNamFromHeader");
				return file_name;
			}

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