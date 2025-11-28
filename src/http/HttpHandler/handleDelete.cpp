#include "HttpHandler.hpp"
#include "../utils.hpp"

HttpResponse HttpHandler::handleDelete(const HttpRequest& req)
{
	HttpResponse res;

	res.setVersion(req.getVersion());
	FileSystem fs(req_config.safePath, req_config);

	if(!req_config.deleteAllowed)
		return handleErrorPages(req, METHOD_NOT_ALLOWED);

	if(fs.exists() == false)
		return handleErrorPages(req, NOT_FOUND);
	if(fs.getPath().getRequestedPath() == "/")
	{
		return handleErrorPages(req, METHOD_NOT_ALLOWED);
	}
	if(req_config.deleteAllowed == false || fs.writable() == false)
		return handleErrorPages(req, FORBIDDEN);

	try
	{
		if (std::remove(fs.getPath().getFullPath().c_str()) != 0)
		{
			error("Error deleting file: " + fs.getPath().getFullPath(), "FileSystem");
		}
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

	finalizeResponse(res, req);
	return res;
}