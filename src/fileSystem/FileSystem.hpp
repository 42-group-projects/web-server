#pragma once

#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ctime>

#include "../src/fileSystem/safePath/SafePath.hpp"
#include "../src/fileSystem/directoryListing/DirectoryListing.hpp"
#include "../src/fileSystem/errorPageGenerator/ErrorPageGenerator.hpp"
#include "../include/globals.hpp"
#include "../src/errorHandling/ErrorWarning.hpp"
#include "enums.hpp"

class FileSystem
{
private:
	SafePath sp;
	size_t size;
	time_t lastModified;
	e_mimeType mimeType;
	bool isExists;
	bool isDirectory;
	bool isDirectoryListing;
	bool isReadable;
	bool isWritable;
	bool isExecutable;
	e_error_page_type isErrorPage;

	std::string directoryListingStr;
	std::string errorPageStr;

	void fillMetadata();
	void fillDirectoryListingMetadata();
	void fillGeneratedErrorPageMetadata(e_status_code code);
	e_mimeType detectMimeType(const SafePath& safePath);

public:
	FileSystem(SafePath path);

	const std::string getFileContents() const;
	void errorPage(e_status_code code);

	const SafePath& getPath() const;
	size_t getSize() const;
	time_t getLastModified() const;
	e_mimeType getMimeType() const;
	bool exists() const;
	bool directory() const;
	bool directoryListing() const;
	bool readable() const;
	bool writable() const;
	bool executable() const;
};

std::ostream& operator<<(std::ostream& os, const FileSystem& file);
