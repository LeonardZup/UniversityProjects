<?php
    require_once "db_connect.php";
    
    session_start();

    if (!isset($_SESSION["logged"])) {
        echo json_encode(["status" => "error", "message" => "Unauthorized"]);
        exit();
    }

    try {
        $stmt = $pdo->prepare("SELECT preset_name, loop_set, active_loops FROM presets WHERE username = :username");
        $stmt->execute(["username" => $_SESSION["username"]]);
        $presets = $stmt->fetchAll();
        
        echo json_encode(["status" => "success", "presets" => $presets]);
    } catch (PDOException $e) {
        echo json_encode(["status" => "error", "message" => "Server error"]);
    }
?>