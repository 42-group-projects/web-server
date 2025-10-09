#include "HttpHandler.hpp"
#include "../utils.hpp"
#include <fstream>
#include <unistd.h>

std::string formatFileName(const HttpRequest &req);

HttpResponse HttpHandler::handlePost(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());
	FileSystem fs(SafePath(req.getUri()));
	LocationConfig location_config = config[fs];
	// DEBUGIING
	// displayLocationConfigDetails(location_config);
	// displayFileSystemInfo(fs);

	// Need to make sure the error handling here is correct
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
		std::string filename = formatFileName(req);

		try
		{
			// Create the full file path
			std::stringstream filepath_ss;
			filepath_ss << location_config.root << "/" << filename;
			std::ofstream outfile(filepath_ss.str().c_str(), std::ios::binary);

			if (!outfile.is_open())
			{
					error("Failed to open file for writing", "HttpHandler::handlePost");
			}
			else
			{
				// Write the body to the file
				outfile << req.getBody();
				outfile.close();

				// set all the response details here
				res.setStatus(CREATED);
				res.setMimeType("text/plain");
				res.setHeader("Location", location_config.upload_store + "/" + filename);
				res.setBody("File uploaded successfully as " + filename + "\n");
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
	std::string filename;

	try
	{
		//check to see if there is a filename in the URI or content-disposition header
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}

	if(filename.empty())
	{
		// generate a unique filename using timestamp and adds file extension based on mime type
		std::stringstream ss;
		e_mimeType mime_enum = getMimeTypeEnum(req.getMimeTypeString());
		std::string ext = getMimeTypeExtention(mime_enum);
		ss << "upload_file_" << time(NULL) << ext;
		filename = ss.str();
	}

	return filename;
}