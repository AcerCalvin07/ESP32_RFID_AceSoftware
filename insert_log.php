<?php
$servername = "localhost";
$username   = "root";
$password   = "";
$dbname     = "it414_db_acesoftware"; 

$mysqli = new mysqli($servername, $username, $password, $dbname);
if ($mysqli->connect_errno) {
    die("Connection failed: " . $mysqli->connect_error);
}

// --- Handle ESP32 POST request ---
if ($_SERVER["REQUEST_METHOD"] === "POST") {
    header('Content-Type: text/plain'); // ESP32 only needs plain text
    $rfid_raw = isset($_POST['rfid_data']) ? $_POST['rfid_data'] : null;

    if ($rfid_raw) {
        $rfid_upper = strtoupper(trim($rfid_raw));

        // Check if RFID exists in rfid_reg
        $stmt = $mysqli->prepare("SELECT rfid_status FROM rfid_reg WHERE UPPER(rfid_data) = ? LIMIT 1");
        $stmt->bind_param('s', $rfid_upper);
        $stmt->execute();
        $res = $stmt->get_result();

        if ($res && $res->num_rows > 0) {
            $row = $res->fetch_assoc();
            $status = (int)$row['rfid_status'];

            // Insert into rfid_logs
            $insert = $mysqli->prepare("
                INSERT INTO rfid_logs (time_log, time_log_12, rfid_data, rfid_status)
                VALUES (NOW(), DATE_FORMAT(NOW(), '%Y-%m-%d %h:%i:%s %p'), ?, ?)
            ");
            $insert->bind_param('si', $rfid_upper, $status);
            $insert->execute();
            $insert->close();

            echo ($status === 1) ? "VALID" : "INACTIVE";

        } else {
            // Not found in rfid_reg → mark with -1
            $status = -1;
            $insert = $mysqli->prepare("
                INSERT INTO rfid_logs (time_log, time_log_12, rfid_data, rfid_status)
                VALUES (NOW(), DATE_FORMAT(NOW(), '%Y-%m-%d %h:%i:%s %p'), ?, ?)
            ");
            $insert->bind_param('si', $rfid_upper, $status);
            $insert->execute();
            $insert->close();

            echo "NOT_FOUND";
        }
        $stmt->close();
    } else {
        echo "NO_DATA";
    }
    $mysqli->close();
    exit; // prevent HTML output for ESP32
}
?>

<!DOCTYPE html>
<html>
<head>
    <title>RFID Logs</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        table { border-collapse: collapse; width: 100%; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }
        th { background-color: #333; color: white; }
        tr:nth-child(even) { background-color: #f2f2f2; }
        tr:hover { background-color: #ddd; }
        .valid { color: green; font-weight: bold; }
        .inactive { color: red; font-weight: bold; }
        .notfound { color: orange; font-weight: bold; }
    </style>
</head>
<body>
    <h2>ACE Software RFID Logs</h2>
    <table>
        <tr>
            <th>ID</th>
            <th>RFID</th>
            <th>Timestamp</th>
            <th>Status</th>
        </tr>
        <?php
        // Fetch logs for browser view
        $result = $mysqli->query("SELECT * FROM rfid_logs ORDER BY time_log DESC LIMIT 50");
        if ($result && $result->num_rows > 0) {
            while ($row = $result->fetch_assoc()) {
                $statusText = "";
                $statusClass = "";
                if ($row['rfid_status'] == 1) {
                    $statusText = "ACTIVE";
                    $statusClass = "valid";
                } elseif ($row['rfid_status'] == 0) {
                    $statusText = "INACTIVE";
                    $statusClass = "inactive";
                } elseif ($row['rfid_status'] == -1) {
                    $statusText = "NOT REGISTERED";
                    $statusClass = "notfound";
                }
                echo "<tr>
                        <td>{$row['id']}</td>
                        <td>{$row['rfid_data']}</td>
                        <td>{$row['time_log_12']}</td>
                        <td class='{$statusClass}'>{$statusText}</td>
                      </tr>";
            }
        } else {
            echo "<tr><td colspan='5'>No logs found</td></tr>";
        }
        $mysqli->close();
        ?>
    </table>
</body>
</html>
