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
            id,
            rfid_data as rfid_tag,
            CASE 
                WHEN rfid_status = 1 THEN 'ACTIVE'
                WHEN rfid_status = 0 THEN 'INACTIVE' 
                ELSE 'NOT_FOUND'
            END as status,
            time_log_12 as timestamp
          FROM rfid_logs 
          ORDER BY time_log DESC 
          LIMIT 50";
          
$result = $mysqli->query($query);

$logs = [];
if ($result && $result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
        $logs[] = $row;
    }
}

echo json_encode($logs);
$mysqli->close();
?>