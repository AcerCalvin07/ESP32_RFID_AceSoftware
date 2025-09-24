RFID IoT Logging System (ESP32 + PHP + MySQL)

This project uses an ESP32, RFID scanner, and a PHP + MySQL backend to log and track RFID scans.
When an RFID tag is scanned:

If it exists in the database and is active → logs as VALID
If it exists but inactive → logs as INACTIVE
If not registered → logs as NOT REGISTERED

Both the ESP32 serial terminal and a web dashboard (insert_log.php) will display the results.

Prerequisites
Hardware:
- ESP32 development board
- RC522 RFID module (or compatible reader)
- RFID tags/cards
- USB cable for programming

Software:

- PlatformIO Extension in VSCode

Arduino libraries:
- WiFi.h (built-in)
- HTTPClient.h (built-in)
- MFRC522 (for RFID reader)

Local server:
- XAMPP

Database:
- MySQL

⚙️ Setup Steps
1. Install Local Server
- Install XAMPP
- Start Apache and MySQL in the control panel.
- Place project files (like insert_log.php) inside the htdocs/ folder (e.g., C:\xampp\htdocs\rfid_project\). (create if there's none)

2. Create Database
- Open http://localhost/phpmyadmin
- Create a database (it414_db_acesoftwaretest).
- Run the following SQL queries:

CREATE TABLE rfid_reg (
    id INT AUTO_INCREMENT PRIMARY KEY,
    rfid_data VARCHAR(50) NOT NULL UNIQUE,
    rfid_status TINYINT(1) NOT NULL DEFAULT 1
);

CREATE TABLE rfid_logs (
    id INT AUTO_INCREMENT PRIMARY KEY,
    time_log DATETIME NOT NULL,
    time_log_12 VARCHAR(30) NOT NULL,
    rfid_data VARCHAR(50) NOT NULL,
    rfid_status INT NOT NULL
);

3. Configure PHP File
- Place insert_log.php into htdocs/rfid_project/.
- Edit the DB credentials if needed:

$servername = "localhost";
$username   = "root";
$password   = "";          // (default for XAMPP is empty)
$dbname     = "it414_db_acesoftwaretest";

- Open in browser: http://localhost/rfid_project/insert_log.php (should show empty logs table)

4. Configure ESP32 Code
- Open VSCode with PlatformIO.
- Install MFRC522 library in PlatformIO Libraries
- Edit the Wi-Fi and server settings in your ESP32 code:

const char* preferred_ssid     = "Cloud Control Network";
const char* preferred_password = "ccv7network";

const char* fallback_ssid      = "STUDENT-CONNECT";     // <-- change
const char* fallback_password  = "IloveUSTP!";     // <-- change

const char* serverUrl = "http://192.168.100.5/rfid_system/insert_log.php"; // <-- change to your PC IP

- Replace YOUR_PC_IP with your computer’s local IP address, e.g. 192.168.1.100.
- Check it with ipconfig (Windows).
- Upload code to ESP32.

5. Test
- Open PlatformIO Serial Monitor at 115200 baud.
- Scan an RFID tag.
- Open browser at http://localhost/rfid_project/insert_log.php
- Logs table should show the scan with proper status.

6. Register RFID Tag
- Insert data inside rfid_reg
- Copy RFID UID
- Set Status to 1 = ACTIVE or 0 = INACTIVE
