#include "HttpHandler.hpp"
#include "../utils.hpp"

HttpResponse HttpHandler::handleGet(const HttpRequest& req)
{
	HttpResponse res;
	res.setVersion(req.getVersion());

	FileSystem fs(SafePath(req.getUri()));
	if(!fs.exists())
		return handleErrorPages(req, NOT_FOUND);
	if(!fs.readable())
		return handleErrorPages(req, FORBIDDEN);
	if(fs.directory())
		return handleErrorPages(req, FORBIDDEN);
	
	res.setStatus(OK);
	res.setMimeType(getMimeTypeString(fs.getMimeType()));
	res.setBody(fs.getFileContents());
	return res;
}