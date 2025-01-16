#!/usr/bin/python3

import cgi
import cgitb

# Enable debugging
cgitb.enable()

def calculate(num1, num2, operator):
    try:
        num1 = float(num1)
        num2 = float(num2)

        if operator == '+':
            return f"{num1 + num2}\0"
        elif operator == '-':
            return f"{num1 - num2}\0"
        elif operator == '*':
            return f"{num1 * num2}\0"
        elif operator == '/':
            if num2 == 0:
                return "Error: Division by zero\0"
            return f"{num1 / num2}\0"
        else:
            return "Error: Unsupported operator\0"
    except ValueError:
        return "Error: Invalid input\0"

# Output HTTP headers
print("Content-Type: text/html")
print()

# Parse input from query parameters
form = cgi.FieldStorage()
num1 = form.getvalue('num1')
num2 = form.getvalue('num2')
operator = form.getvalue('op')

# Validate inputs
if num1 is None or num2 is None or operator is None:
    print("<html><body>")
    print("<h1>Error: Missing parameters</h1>")
    print("<p>Provide parameters: num1, num2, and op (+, -, *, /)</p>")
    print("</body></html>")
else:
    # Perform the calculation
    result = calculate(num1, num2, operator)

    # Return the result as an HTML response
    print("<html><body>\r\n")
    print("<h1>Calculator Result</h1>\r\n")
    print(f"<p>{num1} {operator} {num2} = {result}</p>\r\n")
    print("</body></html>\r\n\r\n\0")
