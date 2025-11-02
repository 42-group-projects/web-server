#include "HttpHandler.hpp"
#include "../utils.hpp"

HttpResponse HttpHandler::handleGet(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());

	FileSystem fs = SafePath(req.getUri());
	LocationConfig location_config = config[fs];
	int code = OK;

	if (g_config[fs.getPath()].redirect_enabled)
	{
		code = g_config[fs.getPath()].redirect_code;
		fs = FileSystem(SafePath(g_config[fs.getPath()].redirect_url));
	}

	if(!fs.exists())
		return handleErrorPages(req, NOT_FOUND);
	if(!fs.readable())
		return handleErrorPages(req, FORBIDDEN);
	if(fs.directory())
		return handleErrorPages(req, FORBIDDEN);

	res.setStatus(getStatusCodeFromInt(code));
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	res.setBody(fs.getFileContents());
	return res;
}