#!/usr/bin/env php
<?php
echo "Content-Type: text/html\n\n";

echo "<html><head><title>CGI Hello</title></head><body>";
echo "<h1>Hello, World! This is PHP!</h1>";

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $name = $_POST['name'] ?? '';
    $message = $_POST['message'] ?? '';

    echo "<h2>Received POST Data</h2>";
    echo "<p>Name: $name</p>";
    echo "<p>Message: $message</p>";
}

echo "</body></html>";
?>
