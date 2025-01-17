#!/usr/bin/python3
import os
import cgi
import cgitb
# Enable debugging
cgitb.enable()
def parse_headers_and_body(input_data):
    """
    Parses input data containing headers and body.
    Extracts headers into environment variables and determines the body content.
    """
    headers, body = input_data.split("\r\n\r\n", 1)
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
    body = parse_headers_and_body(input_data)
    method = os.environ.get("REQUEST_METHOD", "")
    if method == "GET":
        query_string = os.environ.get("QUERY_STRING", "")
        form = cgi.parse_qs(query_string)
    elif method == "POST":
        form = cgi.parse_qs(body)
    else:
        return "Error: Unsupported method"
    num1 = form.get("num1", [None])[0]
    num2 = form.get("num2", [None])[0]
    operator = form.get("op", [None])[0]
    if num1 is None or num2 is None or operator is None:
        return "Error: Missing parameters"
    # Perform the calculation
    result = calculate(num1, num2, operator)
    return result
# Example usage
if __name__ == "__main__":
    import sys
    input_data = sys.stdin.read()  # Read the input string from stdin
    response = handle_request(input_data)
    print(response)  # Output the result as a plain string