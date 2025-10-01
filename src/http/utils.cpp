#include "enums.hpp"
#include "../../include/imports.hpp"
#include "./utils.hpp"

std::string get_method_string(e_method method)
{
	switch (method)
	{
	case GET:
		return "GET";

	case POST:
		return "POST";

	case DELETE:
		return "DELETE";

	default:
		return "UNKNOWN";
	}
}

std:: string get_current_time()
{
	time_t now = time(NULL);
	struct tm *tm_info = gmtime(&now);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
	return std::string(buffer);
}

void displayFileSystemInfo(FileSystem const &fs)
{
	std::cout << "-------FILESYSTEM INFO-------" << std::endl;
	std::cout << "Path: " << fs.getPath().getFullPath() << std::endl;
	std::cout << "Size: " << fs.getSize() << " bytes" << std::endl;
	// std::cout << "Last Modified: " << ctime(&fs.getLastModified());
	std::cout << "MIME Type: " << fs.getMimeType() << std::endl;
	std::cout << "Exists: " << (fs.exists() ? "Yes" : "No") << std::endl;
	std::cout << "Directory: " << (fs.directory() ? "Yes" : "No") << std::endl;
	std::cout << "Readable: " << (fs.readable() ? "Yes" : "No") << std::endl;
	std::cout << "Writable: " << (fs.writable() ? "Yes" : "No") << std::endl;
	std::cout << "Executable: " << (fs.executable() ? "Yes" : "No") << std::endl;
}


