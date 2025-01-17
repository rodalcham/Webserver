#!/usr/bin/python3
import sys
import os
import cgi
import cgitb

# Enable debugging to see tracebacks in the browser
cgitb.enable()

def parse_headers_and_body(input_data):
    """
    Parses the input_data containing headers and body.
    Extracts headers into environment variables and returns the body.
    """
    # Split once on the blank line (\r\n\r\n) between headers and body
    try:
        headers, body = input_data.split("\r\n\r\n", 1)
    except ValueError:
        return "Error: Invalid input format (no blank line separating headers and body)"
    
    # Process headers if necessary (not required for body parsing)
    for line in headers.split("\r\n"):
        if ": " in line:
            key, value = line.split(": ", 1)
            env_key = "HTTP_" + key.upper().replace("-", "_")
            os.environ[env_key] = value
        elif line.startswith("GET") or line.startswith("POST"):
            method, uri, _ = line.split()
            os.environ["REQUEST_METHOD"] = method
            if "?" in uri:
                path, query_string = uri.split("?", 1)
                os.environ["QUERY_STRING"] = query_string
            else:
                path = uri
                os.environ["QUERY_STRING"] = ""
            os.environ["SCRIPT_NAME"] = path
    
    return body

def calculate(num1, num2, operator):
    try:
        num1 = float(num1)
        num2 = float(num2)
        if operator == '+':
            return f"{num1 + num2}"
        elif operator == '-':
            return f"{num1 - num2}"
        elif operator == '*':
            return f"{num1 * num2}"
        elif operator == '/':
            if num2 == 0:
                return "Error: Division by zero"
            return f"{num1 / num2}"
        else:
            return "Error: Unsupported operator"
    except ValueError:
        return "Error: Invalid input"

def handle_request(input_data):
    """
    Processes the input data and returns the calculation result.
    """
    body = parse_headers_and_body(input_data)  # Get the body from the HTTP request
    method = os.environ.get("REQUEST_METHOD", "")
    if method == "GET":
        query_string = os.environ.get("QUERY_STRING", "")
        form = cgi.parse_qs(query_string)
    elif method == "POST":
        # For POST, the body should contain the form data
        print(repr(body));
        form = cgi.parse_qs(body)
        print("here");
    else:
        return "Error: Unsupported method"
    
    # Extract the values from the form data
    num1 = form.get("num1", [None])[0]
    num2 = form.get("num2", [None])[0]
    operator = form.get("op", [None])[0]
    
    if num1 is None or num2 is None or operator is None:
        return "Error: Missing parameters"
    
    # Perform the calculation
    return calculate(num1, num2, operator)

if __name__ == "__main__":
    # 1) Read the entire raw HTTP request from STDIN
    input_data = sys.argv[1]
    
    # 2) Handle the request
    result = handle_request(input_data)
    
    # 3) Output minimal CGI headers + result
    print("Content-Type: text/plain\r\n\r\n", end="")
    print(result)
