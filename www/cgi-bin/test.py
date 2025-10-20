#!/usr/bin/python

import cgi

print("Content-type: text/html\n")
print("<html>")
print("<head>")
print("<title>CGI Example</title>")
print("</head>")
print("<body>")

form = cgi.FieldStorage()
name = form.getvalue("name", "Guest")

print("".format(name))
print("</body>")
print("</html>")