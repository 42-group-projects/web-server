#include "HttpHandler.hpp"
#include "../utils.hpp"

HttpResponse HttpHandler::handleDelete(const HttpRequest& req)
{
	HttpResponse res;

	res.setVersion(req.getVersion());
	FileSystem fs(SafePath(req.getUri()));
	LocationConfig location_config = config[fs];

	displayFileSystemInfo(fs);
	displayLocationConfigDetails(location_config);

	if(fs.exists() == false)
		return handleErrorPages(req, NOT_FOUND);
	if(location_config.deleteAllowed == false || fs.writable() == false || fs.directory() == true)
		return handleErrorPages(req, FORBIDDEN);

	try
	{
		// Attempt to delete the file
		// can i use remove() here?
		if (std::remove(fs.getPath().getFullPath().c_str()) != 0)
			error("Error deleting file: " + fs.getPath().getFullPath(), "FileSystem");
		else
		{
			res.setStatus(NO_CONTENT);
			res.setBody("");
			return res;
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return handleErrorPages(req, INTERNAL_SERVER_ERROR);
	}


	return res;
}