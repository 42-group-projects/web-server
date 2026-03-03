*This project has been created as part of the 42 curriculum by mkakizak/, ccolin/, yuotsubo/.*

# web-server
simple nginx written in c++

## Description

A lightweight HTTP/1.0/HTTP/1.1 web server implementation written in C++. The server handles multiple concurrent connections, serves static files, and supports basic HTTP methods (GET, POST, DELETE). Configuration is managed through an nginx-style config file with support for virtual hosts, CGI execution, and custom error pages.

**Requirements:**
- C++98 compliant compiler
- POSIX-compliant system (Linux/macOS)

## Instructions

**Build:**
```bash
make
```

**Run:**
```bash
./webserv [config_file]
```

You can also execute a test script by running:
**Test:**
```bash
./test/runtest.sh
```

## Resources

**Documentation:**
- [RFC 2612 - HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc2616)
- [nginx configuration documentation](https://nginx.org/en/docs/)

**Use of Artificial Intelligence:**
AI tools were used strictly as technical assistants during development.
AI was not used to generate the full implementation of the project.
All architectural design, integration, and code implementation were written and structured manually.
AI functioned as a clarification and diagnostic tool, similar to consulting documentation or asking a peer for guidance.

## Testing suit
- [Webserver tester made fellow 42 student](https://github.com/ulsgks/webserv-tester)
