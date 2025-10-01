# File system / Static html

this should be pretty easy…

### **Primary Responsibilities**

Serve static files safely and efficiently, handle file operations, and manage server configuration.

### **Key Components to Build**

### **1. Safe File Server**

```cpp
class FileServer {
    std::string document_root;

public:
    FileServer(const std::string& root);
    HttpResponse serve_file(const std::string& requested_path);
    bool file_exists(const std::string& path);
    bool is_safe_path(const std::string& path);

private:
    std::string resolve_path(const std::string& requested_path);
    std::string read_file_content(const std::string& full_path);
};

```

**Tasks:**

- Resolve requested paths to actual file paths
- Prevent directory traversal attacks (`../../../etc/passwd`)
- Handle index files (serve `index.html` for `/` requests)
- Check file permissions and existence
- Read file content efficiently

### **2. Directory Listing (Optional Feature)**

```cpp
class DirectoryHandler {
public:
    HttpResponse generate_directory_listing(const std::string& dir_path,
                                          const std::string& url_path);

private:
    std::string generate_html_listing(const std::vector<FileInfo>& files);
};

```

**Tasks:**

- Generate HTML directory listings when no index file exists
- Sort files/directories appropriately
- Show file sizes and modification dates
- Make listings clickable/navigable

### **3. File Information & Caching**

```cpp
struct FileInfo {
    std::string full_path;
    size_t size;
    time_t last_modified;
    std::string mime_type;
    bool exists;
    bool is_directory;
};

class FileCache {
    std::map<std::string, CachedFile> cache;
    size_t max_cache_size;

public:
    FileInfo get_file_info(const std::string& path);
    std::string get_cached_content(const std::string& path);
    void cache_file(const std::string& path, const std::string& content);
};

```

**Tasks:**

- Get file metadata using `stat()`
- Cache frequently accessed files in memory
- Handle cache invalidation (check modification time)
- Implement cache size limits

### **4. Configuration Manager**

```cpp
class ServerConfig {
public:
    int port;
    std::string document_root;
    std::vector<std::string> index_files;
    size_t max_file_size;
    bool enable_directory_listing;

    bool load_config(const std::string& config_file);

private:
    void parse_config_line(const std::string& line);
};

```

**Tasks:**

- Parse configuration file (simple key=value format)
- Set default values for all settings
- Validate configuration values
- Handle missing config file gracefully

### **5. Error Page Generator**

```cpp
class ErrorPages {
public:
    std::string generate_404_page(const std::string& requested_path);
    std::string generate_500_page();
    std::string generate_403_page();

private:
    std::string load_custom_error_page(int status_code);
    std::string generate_default_error_page(int status_code, const std::string& message);
};

```

**Tasks:**

- Generate HTML error pages
- Support custom error page files (404.html, 500.html)
- Create reasonable default error pages
- Include useful error information

### **Security Considerations:**

```cpp
class PathValidator {
public:
    static bool is_safe_path(const std::string& path);
    static std::string normalize_path(const std::string& path);

private:
    static bool contains_traversal(const std::string& path);
    static std::string resolve_symlinks(const std::string& path);
};

```

**Tasks:**

- Block `../` directory traversal attempts
- Normalize paths (`/./path` → `/path`)
- Handle symbolic links safely
- Restrict access to document root only

### **Key Functions You'll Use:**

`open`, `read`, `close`, `stat`, `access`, `opendir`, `readdir`, `chdir` (to validate paths)

### **File Operations to Handle:**

- Text files (HTML, CSS, JavaScript, JSON)
- Binary files (images, videos, executables)
- Large files (streaming vs loading into memory)
- Non-existent files (404 handling)
- Permission denied (403 handling)

### **Testing Strategy:**

- Test various file types and extensions
- Try directory traversal attacks: `GET /../../../etc/passwd`
- Test large files and binary files
- Verify MIME types are set correctly
- Test with missing files and directories

### **Deliverables:**

1. Safe file serving with security checks
2. Support for all common web file types
3. Configuration system for server settings
4. Error page generation
5. Optional: File caching for performance
6. Optional: Directory listing generation