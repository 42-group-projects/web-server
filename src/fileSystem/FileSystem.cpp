#include "../src/fileSystem/FileSystem.hpp"
#include "../include/globals.hpp"
#include "../src/errorHandling/ErrorWarning.hpp"

FileSystem::FileSystem(SafePath path) : fullPath(path)
{
	struct stat sb;

	if (stat(fullPath.getFullPath().c_str(), &sb) == 0)
	{
		isExists = true;
		size = sb.st_size;
		lastModified = sb.st_mtime;
		mimeType = detectMimeType(path);
		isDirectory = S_ISDIR(sb.st_mode);
		isReadable = (access(fullPath.getFullPath().c_str(), R_OK) == 0);
		isWritable = (access(fullPath.getFullPath().c_str(), W_OK) == 0);
		isExecutable = (access(fullPath.getFullPath().c_str(), X_OK) == 0);
	}
	else
	{
		std::cout << "File does not exist: " << fullPath.getFullPath() << std::endl;
		isExists = false;
		size = 0;
		lastModified = 0;
		mimeType = NO_FILE;
		isDirectory = false;
		isReadable = false;
		isWritable = false;
		isExecutable = false;
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

const std::string FileSystem::getFileContents()
{
	std::ifstream file(fullPath.getFullPath().c_str(), std::ios::binary);

	if (!file)
		error("Couldn't open file '" + fullPath.getFullPath() + "'", "File system");

	std::ostringstream oss;
	oss << file.rdbuf();
	return oss.str();
}

const SafePath& FileSystem::getPath() const { return fullPath; }
size_t FileSystem::getSize() const { return size; }
time_t FileSystem::getLastModified() const { return lastModified; }
e_mimeType FileSystem::getMimeType() const { return mimeType; }
bool FileSystem::exists() const { return isExists; }
bool FileSystem::directory() const { return isDirectory; }
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
	os << "Readable: " << file.readable() << std::endl;
	os << "Writable: " << file.writable() << std::endl;
	os << "Executable: " << file.executable() << std::endl << std::endl;
	os << g_config[file.getPath()] << std::endl;
	return os;
}
