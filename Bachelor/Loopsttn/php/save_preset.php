<?php
    require_once "db_connect.php";
    
    session_start();

    define("PRESET_REGEX", "/^[a-zA-Z0-9._]{1,20}$/");

    if (!isset($_SESSION["logged"])) {
        echo json_encode(["status" => "error", "message" => "Unauthorized"]);
        exit();
    }

    $data = json_decode(file_get_contents("php://input"), true);

    if (isset($data["presetName"], $data["loopSet"], $data["activeLoops"])) {
        $username = $_SESSION["username"];
        $presetName = $data["presetName"];

        if (!preg_match(PRESET_REGEX, $presetName)) {
            echo json_encode(["status" => "error", "message" => "Invalid preset name"]);
            exit();
        }

        try {
            $countStmt = $pdo->prepare("SELECT COUNT(*) FROM presets WHERE username = :username");
            $countStmt->execute(["username" => $username]);
            if ($countStmt->fetchColumn() >= 10) {
                echo json_encode(["status" => "error", "message" => "Preset limit reached"]);
                exit();
            }

            $check = $pdo->prepare("SELECT id FROM presets WHERE username = :username AND preset_name = :preset_name");
            $check->execute(["username" => $username, "preset_name" => $presetName]);
            if ($check->fetch()) {
                echo json_encode(["status" => "error", "message" => "Name already in use"]);
                exit();
            }

            $stmt = $pdo->prepare("INSERT INTO presets (username, loop_set, preset_name, active_loops) VALUES (:username, :loop_set, :preset_name, :active_loops)");
            $stmt->execute([
                "username" => $username, 
                "loop_set" => $data["loopSet"], 
                "preset_name" => $presetName, 
                "active_loops" => json_encode($data["activeLoops"])
            ]);

            echo json_encode(["status" => "success"]);
        } catch (PDOException $e) {
            echo json_encode(["status" => "error", "message" => "Server error"]);
        }
    } else {
        echo json_encode(["status" => "error", "message" => "Invalid data"]);
    }
?>