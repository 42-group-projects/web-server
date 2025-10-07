#include "HttpHandler.hpp"
#include "../utils.hpp"
#include <fstream>
#include <unistd.h>


HttpResponse HttpHandler::handlePost(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());
	FileSystem fs(SafePath(req.getUri()));
	LocationConfig location_config = config[fs];
	displayLocationConfigDetails(location_config);
	displayFileSystemInfo(fs);

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
			std::stringstream ss;
			//TDO: make sure this is unique enough with out uing time();
			ss << "upload_file_" << time(NULL) << ".txt";
			filename = ss.str();
		}
		
		try
		{	
			std::cout << "___________Inside try block for file upload_______________" << std::endl;

			// Get current working directory
			char cwd[1024];
			if (getcwd(cwd, sizeof(cwd)) != NULL)
			{
				std::cout << "Current working directory: " << cwd << std::endl;
			}
			
			std::cout << "Location config .root " << location_config.root << std::endl;
			
			//TODO: need to change back to original working directory after upload is done
			// Change to the upload directory
			if(chdir(location_config.root.c_str()) != 0)
			{
				std::cerr << "Failed to change directory to " << location_config.root << std::endl;
				return handleErrorPages(req, INTERNAL_SERVER_ERROR);
			}
			
			//opening/creating thie file here
			std::ofstream outfile(filename.c_str(), std::ios::binary);
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