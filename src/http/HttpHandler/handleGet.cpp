#include "HttpHandler.hpp"
#include "../utils.hpp"

// std::string decodeUri(const std::string& encoded_uri);

HttpResponse HttpHandler::handleGet(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());
	FileSystem fs(req_config.safePath, req_config);

	int code = OK;

	if (!req_config.getAllowed && !req_config.autoindex)
	{
		return handleErrorPages(req, METHOD_NOT_ALLOWED);
	}
	if(!fs.exists())
		return handleErrorPages(req, NOT_FOUND);
	if(!fs.readable())
		return handleErrorPages(req, FORBIDDEN);

	res.setStatus(getStatusCodeFromInt(code));
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	res.setBody(fs.getFileContents());

	return res;
}
