#include "../src/fileSystem/FileSystem.hpp"

FileSystem::FileSystem(SafePath sp, t_request_config& conf) : sp(sp)
{
	directoryListingStr = "";
	errorPageStr = "";
	isErrorPage = NOT_ERROR_PAGE;
	fillMetadata();
	if (isDirectory)
	{
		if (!conf.index.empty())
		{
			std::string newPath;

			if (sp.getRequestedPath()[sp.getRequestedPath().size() - 1] != '/')
				newPath = sp.getFullPath() + "/" + conf.index;
			else
				newPath = sp.getFullPath() + conf.index;

			SafePath backup = sp;

			this->sp.setFullPath(newPath);
			fillMetadata();
			if (!isExists && conf.autoindex)
			{
				sp = backup;
				fillDirectoryListingMetadata();
			}
		}
		else if (conf.autoindex)
			fillDirectoryListingMetadata();
	}
}

void FileSystem::fillDirectoryListingMetadata()
{
	DirectoryListing listing(sp);
	directoryListingStr = listing.getHtml();
	size = directoryListingStr.size();
	mimeType = TEXT_HTML;
	isReadable = true;
	isWritable = false;
	isExecutable = false;
	isDirectoryListing = true;
}

void FileSystem::fillGeneratedErrorPageMetadata(e_status_code code)
{
	ErrorPageGenerator errorPage(code);
	errorPageStr = errorPage.getHtml();
	lastModified = std::time(NULL);
	size = errorPageStr.size();
	mimeType = TEXT_HTML;
	isReadable = true;
	isWritable = false;
	isExecutable = false;
	isDirectoryListing = false;
}

void FileSystem::fillMetadata()
{
	struct stat sb;

	if (stat(sp.getFullPath().c_str(), &sb) == 0)
	{
		isDirectory = S_ISDIR(sb.st_mode);
		isExists = true;
		size = sb.st_size;
		lastModified = sb.st_mtime;
		mimeType = detectMimeType(sp);
		isReadable = (access(sp.getFullPath().c_str(), R_OK) == 0);
		isWritable = (access(sp.getFullPath().c_str(), W_OK) == 0);
		isExecutable = (access(sp.getFullPath().c_str(), X_OK) == 0);
		isDirectoryListing = false;
	}
	else
	{
		isExists = false;
		size = 0;
		lastModified = 0;
		mimeType = NO_FILE;
		isDirectory = false;
		isReadable = false;
		isWritable = false;
		isExecutable = false;
		isDirectoryListing = false;
	}
}

e_mimeType FileSystem::detectMimeType(const SafePath& safePath)
{
	std::string path = safePath;
	std::transform(path.begin(), path.end(), path.begin(), ::tolower);
	std::string extension;
	size_t dot = path.rfind('.');

	if (dot != std::string::npos)
		extension = path.substr(dot);

	if (extension == ".html" || extension == ".htm")
		return TEXT_HTML;

	if (extension == ".css")
		return TEXT_CSS;

	if (extension == ".js")
		return APPLICATION_JAVASCRIPT;

	if (extension == ".json")
		return APPLICATION_JSON;

	if (extension == ".txt")
		return TEXT_PLAIN;

	if (extension == ".png")
		return IMAGE_PNG;

	if (extension == ".jpg" || extension == ".jpeg")
		return IMAGE_JPEG;

	if (extension == ".gif")
		return IMAGE_GIF;

	if (extension == ".pdf")
		return APPLICATION_PDF;

	if (extension == ".svg")
		return IMAGE_SVG;

	return APPLICATION_OCTET_STREAM;
}

const std::string FileSystem::getFileContents() const
{
	if (isDirectoryListing)
		return directoryListingStr;

	if (isErrorPage == GENERATED_ERROR_PAGE)
		return errorPageStr;

	std::ifstream file(sp.getFullPath().c_str(), std::ios::binary);


	// std::cout << "Opening file: " << sp.getFullPath() << std::endl;

	if (!file)
		warning("Couldn't open file '" + sp.getFullPath() + "'", "File system");

	std::ostringstream oss;
	oss << file.rdbuf();
	return oss.str();
}

void FileSystem::errorPage(e_status_code code, t_request_config& conf)
{
	std::string errorPagePath;
	const std::map<int, std::string>& errorPages = conf.error_pages;
	std::map<int, std::string>::const_iterator it = errorPages.find(static_cast<int>(code));

	if (it != errorPages.end())
	{
		errorPagePath = it->second;
		isErrorPage = CONFIG_ERROR_PAGE;
		SafePath backup = sp;

		try
		{
			SafePath newPath(errorPagePath, conf, true);
			sp = newPath;
			fillMetadata();

			if (!isExists)
				throw std::runtime_error("Error page does not exist");
		}
		catch (const std::runtime_error& e)
		{
			sp = backup;
			isErrorPage = GENERATED_ERROR_PAGE;
			fillGeneratedErrorPageMetadata(code);
		}
	}
	else
	{
		isErrorPage = GENERATED_ERROR_PAGE;
		fillGeneratedErrorPageMetadata(code);
	}
}

const SafePath& FileSystem::getPath() const { return sp; }
size_t FileSystem::getSize() const { return size; }
time_t FileSystem::getLastModified() const { return lastModified; }
e_mimeType FileSystem::getMimeType() const { return mimeType; }
bool FileSystem::exists() const { return isExists; }
bool FileSystem::directory() const { return isDirectory; }
bool FileSystem::directoryListing() const { return isDirectoryListing; }
bool FileSystem::readable() const { return isReadable; }
bool FileSystem::writable() const { return isWritable; }
bool FileSystem::executable() const { return isExecutable; }

std::ostream& operator<<(std::ostream& os, const FileSystem& file)
{
	os << "Path: " << file.getPath().getFullPath() << std::endl;
	os << "Requested path: " << file.getPath().getRequestedPath() << std::endl;
	os << "Size: " << file.getSize() << " bytes\n";
	os << "Last modified: " << file.getLastModified() << std::endl;
	os << "MIME type: " << file.getMimeType() << std::endl;
	os << "Exists: " << file.exists() << std::endl;
	os << "Directory: " << file.directory() << std::endl;
	os << "Directory listing: " << file.directoryListing() << std::endl;
	os << "Readable: " << file.readable() << std::endl;
	os << "Writable: " << file.writable() << std::endl;
	os << "Executable: " << file.executable() << std::endl << std::endl;
	os << "file contents:" << std::endl << file.getFileContents() << std::endl;
	return os;
}
