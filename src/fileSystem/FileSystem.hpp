#pragma once

#include <string>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../src/fileSystem/SafePath.hpp"
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

	std::string directoryListingStr;

	void fillMetadata();
	void fillDirectoryListingMetadata();
	e_mimeType detectMimeType(const SafePath& safePath);

public:
	FileSystem(SafePath path);

	const std::string getFileContents() const;
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
