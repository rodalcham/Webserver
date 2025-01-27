#!/usr/bin/python3
import sys
import os
import cgitb

# Enable debugging to see tracebacks in the browser
cgitb.enable()

def parse_headers_and_body(input_data):
    """
    Parses the input_data containing headers and body.
    Extracts headers into environment variables and returns the body.
    """
    try:
        headers, body = input_data.split("\r\n\r\n", 1)
    except ValueError:
        return None, "Error: Invalid input format (no blank line separating headers and body)"
    
    # Process headers (optional for this script)
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
    
    return body, None

def parse_calculation_body(body):
    """
    Parses a raw body string like '2+2' into num1, operator, and num2.
    """
    # Match a format like `2+2`, `10-5`, etc.
    import re
    match = re.match(r"(\d+)([+\-*/])(\d+)", body.strip())
    if not match:
        return None, None, None, "Error: Invalid calculation format. Use the format num1+num2 (e.g., 2+2)."
    num1, operator, num2 = match.groups()
    return num1, operator, num2, None

def calculate(num1, num2, operator):
    """
    Performs the calculation based on the given operator and operands.
    """
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
    body, error = parse_headers_and_body(input_data)
    if error:
        return error
    
    method = os.environ.get("REQUEST_METHOD", "")
    if method != "POST":
        return "Error: Only POST method is supported"

    # Parse the calculation directly from the body (raw text)
    num1, operator, num2, parse_error = parse_calculation_body(body)
    if parse_error:
        return parse_error
    
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