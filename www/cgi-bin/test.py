#!/usr/bin/env python3
from datetime import datetime

print("Content-Type: text/html")
print()
print(f"<html><body>")
print(f"<h1>Current Time</h1>")
print(f"<p>{datetime.now()}</p>")
print(f"</body></html>")
