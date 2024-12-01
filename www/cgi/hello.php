#!/usr/bin/env php
<?php
header("Content-Type: text/html");

echo "<html>";
echo "<head><title>CGI Hello</title></head>";
echo "<body>";
echo "<h1>Hello, World! This is PHP!</h1>";

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    echo "<h2>POST Data</h2>";
    echo "<pre>";
    print_r($_POST);
    echo "</pre>";
}

echo "</body>";
echo "</html>";
?>
