#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\n")
print("<html><head><title>CGI Hello</title></head><body>")
print("<h1>Hello, World! This is Python!</h1>")

if os.environ.get("REQUEST_METHOD") == "POST":
    print("<h2>POST Data</h2>")
    print("<pre>")
    print(sys.stdin.read())
    print("</pre>")

print("</body></html>")
