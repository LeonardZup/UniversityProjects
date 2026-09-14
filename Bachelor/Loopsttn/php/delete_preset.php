<?php
    require_once "db_connect.php";
    
    session_start();

    if (!isset($_SESSION["logged"])) {
        echo json_encode(["status" => "error", "message" => "Unauthorized"]);
        exit();
    }

    $data = json_decode(file_get_contents("php://input"), true);

    if (isset($data["presetName"]) && !empty($data["presetName"])) {
        try {
            $stmt = $pdo->prepare("DELETE FROM presets WHERE username = :username AND preset_name = :preset_name");
            $stmt->execute(["username" => $_SESSION["username"], "preset_name" => $data["presetName"]]);
            
            echo json_encode(["status" => "success"]);
        } catch (PDOException $e) {
            echo json_encode(["status" => "error", "message" => "Database error"]);
        }
    } else {
        echo json_encode(["status" => "error", "message" => "Missing data"]);
    }
?>