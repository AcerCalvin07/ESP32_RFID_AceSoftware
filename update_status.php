<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

if ($_SERVER['REQUEST_METHOD'] == 'OPTIONS') {
    exit(0);
}

$servername = "localhost";
$username   = "root";
$password   = "";
$dbname     = "it414_db_acesoftware"; 

$mysqli = new mysqli($servername, $username, $password, $dbname);
if ($mysqli->connect_errno) {
    die(json_encode(["success" => false, "error" => "Connection failed"]));
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $input = json_decode(file_get_contents('php://input'), true);
    $rfid_tag = $input['rfid_tag'] ?? '';
    $new_status = $input['new_status'] ?? '';
    
    if ($rfid_tag && ($new_status === 0 || $new_status === 1)) {
        $update_stmt = $mysqli->prepare("UPDATE rfid_reg SET rfid_status = ? WHERE rfid_data = ?");
        $update_stmt->bind_param('is', $new_status, $rfid_tag);
        
        if ($update_stmt->execute()) {
            $log_stmt = $mysqli->prepare("
                INSERT INTO rfid_logs (time_log, time_log_12, rfid_data, rfid_status)
                VALUES (NOW(), DATE_FORMAT(NOW(), '%Y-%m-%d %h:%i:%s %p'), ?, ?)
            ");
            $log_stmt->bind_param('si', $rfid_tag, $new_status);
            $log_stmt->execute();
            $log_stmt->close();
            
            echo json_encode(["success" => true, "message" => "Status updated"]);
        } else {
            echo json_encode(["success" => false, "error" => "Update failed"]);
        }
        
        $update_stmt->close();
    } else {
        echo json_encode(["success" => false, "error" => "Invalid data"]);
    }
} else {
    echo json_encode(["success" => false, "error" => "Invalid request method"]);
}

$mysqli->close();
?>