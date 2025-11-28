#!/usr/bin/python


print("Content-type: text/html\n")
print("<html>")
print("<head>")
print("<title>CGI Example</title>")
print("</head>")
print("<body>")

name = format.getvalue("name", "Guest")

print("".format(name))
print("</body>")
print("</html>")