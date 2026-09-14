<?php
    define("DB_HOST", "localhost");
    define("DB_NAME", "zuppa_635359");
    define("DB_USERNAME", "root");
    define("DB_PASSWORD", "");

    try {
        $connString = "mysql:host=" . DB_HOST . ";dbname=" . DB_NAME . ";charset=utf8mb4";
        $username = DB_USERNAME;
        $password = DB_PASSWORD;

        $pdo = new PDO($connString, $username, $password);

        $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    } catch (PDOException $e){
        die("Connection error");
    }
?>