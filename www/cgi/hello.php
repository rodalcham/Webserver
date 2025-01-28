#!/usr/bin/env php
<?php
// 1) Ensure we have argv[1]:
if ($argc < 2 || empty($argv[1])) {
    echo "Error: No input data provided.\n";
    exit(1);
}

// 2) The full raw HTTP request is in argv[1].
$input = $argv[1];

// 3) Separate headers from body by "\r\n\r\n"
$parts = explode("\r\n\r\n", $input, 2);

// If we have two parts, the second is the body; otherwise use the entire input as the body.
if (count($parts) === 2) {
    list($rawHeaders, $rawBody) = $parts;
} else {
    // If there's no blank line, everything is 'body'
    $rawBody = $input;
}

// 4) Output minimal CGI headers
header("Content-Type: text/html");
echo "\r\n";  // End of headers

// 5) Display the body in a "Hello ...!" message
// echo "<html><head><title>Hello CGI</title></head><body>";
echo "Hello " . htmlspecialchars($rawBody) . "!\r\n";
// echo "</body></html>";
