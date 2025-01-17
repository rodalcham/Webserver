#!/usr/bin/env php
<?php
// Parse input string from stdin
$input = file_get_contents("php://stdin");
// Split the input into headers and body
list($rawHeaders, $rawBody) = explode("\r\n\r\n", $input, 2);
// Parse headers into an associative array
$headers = [];
foreach (explode("\r\n", $rawHeaders) as $line) {
    if (strpos($line, ":") !== false) {
        list($key, $value) = explode(":", $line, 2);
        $headers[trim($key)] = trim($value);
    }
}
// Extract the REQUEST_METHOD from headers
$requestMethod = $headers['Request-Method'] ?? '';
$_SERVER['REQUEST_METHOD'] = $requestMethod;
// Extract other environment variables from headers
$_SERVER['CONTENT_TYPE'] = $headers['Content-Type'] ?? '';
$_SERVER['CONTENT_LENGTH'] = $headers['Content-Length'] ?? '';
// Handle GET method
if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    parse_str(parse_url($headers['Query-String'] ?? '', PHP_URL_QUERY), $_GET);
} elseif ($_SERVER['REQUEST_METHOD'] === 'POST') {
    parse_str($rawBody, $_POST);
} else {
    echo "HTTP/1.1 405 Method Not Allowed\r\n";
    echo "Content-Type: text/plain\r\n\r\n";
    echo "Error: Unsupported request method.";
    exit;
}
// Output the response
header("Content-Type: text/html");
echo "<html><head><title>CGI Hello</title></head><body>";
echo "<h1>Hello, World! This is PHP!</h1>";
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $name = $_POST['name'] ?? '';
    $message = $_POST['message'] ?? '';
    echo "<h2>Received POST Data</h2>";
    echo "<p>Name: $name</p>";
    echo "<p>Message: $message</p>";
} elseif ($_SERVER['REQUEST_METHOD'] === 'GET') {
    echo "<h2>Received GET Data</h2>";
    foreach ($_GET as $key => $value) {
        echo "<p>$key: $value</p>";
    }
}
echo "</body></html>";






