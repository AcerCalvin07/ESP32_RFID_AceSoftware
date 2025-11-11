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
    die(json_encode(["error" => "Connection failed: " . $mysqli->connect_error]));
}

$query = "SELECT 
            r.id,
            r.rfid_data as rfid_tag,
            'User' as user_name,
            r.rfid_status as is_active,
            COALESCE(
                (SELECT MAX(time_log_12) 
                 FROM rfid_logs l 
                 WHERE l.rfid_data = r.rfid_data),
                'Never'
            ) as last_scan
          FROM rfid_reg r
          ORDER BY last_scan DESC";
          
$result = $mysqli->query($query);

$status = [];
if ($result && $result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $status[] = $row;
    }
}

echo json_encode($status);
$mysqli->close();
?>