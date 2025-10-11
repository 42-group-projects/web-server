#include "DirectoryListing.hpp"

const std::string DirectoryListing::liStart = "<li><a href=\"";
const std::string DirectoryListing::hrefEnd = "\">";
const std::string DirectoryListing::liEnd = "</a></li>";
const std::string DirectoryListing::htmlTemplate =
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"\t<meta charset=\"UTF-8\">\n"
"\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
"\t<title>TITLE directory listing</title>\n"
"</head>\n"
"<body>\n"
"\t<h1>H1 directory listing</h1>\n"
"\t<ul>\n"
"LINK_LIST"
"\t</ul>\n"
"</body>\n"
"</html>\n";

DirectoryListing::DirectoryListing(SafePath path) : path(path)
{
    std::string linkList = createLinkList(listDir(path.getFullPath()));
    html = htmlTemplate;
    size_t pos = html.find("LINK_LIST");
    if (pos != std::string::npos)
        html.replace(pos, 9, linkList);

    pos = html.find("TITLE");
    if (pos != std::string::npos)
        html.replace(pos, 5, path.getRequestedPath());

    pos = html.find("H1");
    if (pos != std::string::npos)
        html.replace(pos, 2, path.getRequestedPath());
}

std::vector<FileEntry> DirectoryListing::listDir(const std::string& path) {
    std::vector<FileEntry> entries;
    DIR* dir = opendir(path.c_str());
    if (!dir) return entries;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..")
            continue;

        std::string fullPath = path + "/" + name;
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0)
        {
            FileEntry e;
            e.name = name;
            e.isDir = (st.st_mode & S_IFDIR) != 0;
            entries.push_back(e);
        }
    }
    closedir(dir);
    return entries;
}

std::string DirectoryListing::createLinkList(std::vector<FileEntry> entries)
{
    std::string list;
    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].isDir)
            list += "\t\t" + liStart + entries[i].name + hrefEnd + entries[i].name + "/" + liEnd + "\n";
        else
            list += "\t\t" + liStart + entries[i].name + hrefEnd + entries[i].name + liEnd + "\n";
    }
    return list;
}

const std::string& DirectoryListing::getHtml() const { return html; }
