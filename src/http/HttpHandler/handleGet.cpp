#include "HttpHandler.hpp"
#include "../utils.hpp"

HttpResponse HttpHandler::handleGet(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());
	FileSystem fs(req_config.safePath, req_config);

	int code = OK;

	if(!fs.exists())
	{
		return handleErrorPages(req, NOT_FOUND);
	}
	if(!fs.readable())
		return handleErrorPages(req, FORBIDDEN);
	if(fs.directory())
		return handleErrorPages(req, FORBIDDEN);

	if (req_config.redirect_enabled)
	{
		code = req_config.redirect_code;
		// redirect needs to be handled here
		// TODO:need to make a new req_config for the new redirect path...
		fs = FileSystem(req_config.safePath, req_config);
	}


	res.setStatus(getStatusCodeFromInt(code));
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	res.setBody(fs.getFileContents());

	return res;
}