<?php
header('Content-Type: application/json');  // JSON

$servername = "localhost";
$username   = "root";
$password   = "";
$dbname     = "it414_db_acesoftware";

$mysqli = new mysqli($servername, $username, $password, $dbname);
if ($mysqli->connect_errno) {
  die(json_encode(["status" => "ERROR", "message" => $mysqli->connect_error]));
}

// GET and POST
$rfid_raw = isset($_REQUEST['rfid_data']) ? $_REQUEST['rfid_data'] : null;

if ($rfid_raw) {
    $rfid_upper = strtoupper(trim($rfid_raw));

    // Check if RFID exists in ditabis
    $stmt = $mysqli->prepare("SELECT rfid_status FROM rfid_reg WHERE UPPER(rfid_data) = ? LIMIT 1");
    $stmt->bind_param('s', $rfid_upper);
    $stmt->execute();
    $res = $stmt->get_result();

    if ($res && $res->num_rows > 0) {
        $row = $res->fetch_assoc();
        $current_status = (int)$row['rfid_status'];

        // RFID Logs to DB
        $insert = $mysqli->prepare("
            INSERT INTO rfid_logs (time_log, time_log_12, rfid_data, rfid_status)
            VALUES (NOW(), DATE_FORMAT(NOW(), '%Y-%m-%d %h:%i:%s %p'), ?, ?)
        ");
        $insert->bind_param('si', $rfid_upper, $current_status);
        $insert->execute();
        $insert->close();

        echo json_encode([
            "status" => ($current_status == 1) ? "ACTIVE" : "INACTIVE",
            "rfid"   => $rfid_upper
        ]);
    } else {
        // If tag is not registered
        $status = -1;
        $insert = $mysqli->prepare("
            INSERT INTO rfid_logs (time_log, time_log_12, rfid_data, rfid_status)
            VALUES (NOW(), DATE_FORMAT(NOW(), '%Y-%m-%d %h:%i:%s %p'), ?, ?)
        ");
        $insert->bind_param('si', $rfid_upper, $status);
        $insert->execute();
        $insert->close();

        echo json_encode([
            "status" => "NOT_REGISTERED",
            "rfid"   => $rfid_upper
        ]);
    }

    $stmt->close();
} else {
  echo json_encode(["status" => "NO_DATA"]);
}

$mysqli->close();
?>