#!/usr/bin/env python3

import os
import sys
from urllib.parse import parse_qs

# Print the Content-Type header
print("Content-Type: text/html\n")

# HTML structure
print("<html><head><title>CGI Hello</title></head><body>")
print("<h1>Hello, World! This is Python!</h1>")

# Debug: Print environment variables (remove in production)
print("<h2>Environment Variables</h2>")
print("<pre>")
for key, value in os.environ.items():
    print(f"{key}={value}")
print("</pre>")

# Check the request method
if os.environ.get("REQUEST_METHOD") == "POST":
    # Read POST data
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    post_data = sys.stdin.read(content_length)
    
    # Debug: Print POST data
    print("<h2>Debug POST Data</h2>")
    print(f"<pre>{post_data}</pre>")
    
    # Parse POST data
    parsed_data = parse_qs(post_data)
    name = parsed_data.get("name", [""])[0]
    message = parsed_data.get("message", [""])[0]
    
    # Display parsed POST data
    print("<h2>Received POST Data</h2>")
    print(f"<p>Name: {name}</p>")
    print(f"<p>Message: {message}</p>")
else:
    print("<p>No POST data received.</p>")

# Close HTML structure
print("</body></html>")
