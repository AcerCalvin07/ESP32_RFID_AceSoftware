RFID IoT Logging System (ESP32 + PHP + MySQL)

This project uses an ESP32, RFID scanner, and a PHP + MySQL backend to log and track RFID scans.
When an RFID tag is scanned:

If it exists in the database and value = 1, it logs as ACTIVE
If it exists but value = 0, it logs as INACTIVE
If not registered → logs as NOT REGISTERED

Both the ESP32 serial terminal and a web dashboard (insert_log.php) will display the results.
The dashboard now automatically reloads every 1 second for real-time updates.

Prerequisites

Hardware:
- ESP32 development board
- RC522 RFID module
- RFID tags or cards
- Micro USB cable
- Breadboard
- Jumper Wires

Software:
- PlatformIO Extension in VSCode
- MFRC522 Library
- WiFi.h (built-in)
- HTTPClient.h (built-in)
- XAMPP (for local server)
- MySQL Database

Setup Steps:
- Install XAMPP
- Start Apache and MySQL from the XAMPP Control Panel.
- Create a new folder inside htdocs named “rfid_system”.
- Place insert_log.php inside that folder.
- Create the Database
- Open phpMyAdmin at http://localhost/phpmyadmin
- Create a database named it414_db_acesoftwaretest
- Run the following SQL commands:

CREATE TABLE rfid_reg (
id INT AUTO_INCREMENT PRIMARY KEY,
rfid_data VARCHAR(50) NOT NULL UNIQUE,
rfid_status TINYINT(1) NOT NULL DEFAULT 1
);

CREATE TABLE rfid_logs (
id INT AUTO_INCREMENT PRIMARY KEY,
rfid_data VARCHAR(50) NOT NULL,
rfid_status VARCHAR(50) NOT NULL,
time_log DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

Configure the PHP File (insert_log.php)
- Edit the database credentials inside insert_log.php if needed: localhost, root, empty password, and database name it414_db_acesoftwaretest.
- Save the file in C:\xampp\htdocs\rfid_system\
- Open the file in your browser: http://localhost/rfid_system/insert_log.php

Configure the ESP32 Code (main.cpp)
- Open the project in VSCode with PlatformIO.
- Edit the Wi-Fi credentials and server URL in the code.
- Replace the server IP with your computer’s local IP (use ipconfig to find it).
- Upload the code to your ESP32.
- Open the Serial Monitor at 115200 baud.
- Test the System
- Scan an RFID tag.
- Check the Serial Monitor for feedback.
- Open http://localhost/rfid_system/insert_log.php in a browser.
- The scanned data should appear automatically and refresh every 1 second.

Register RFID Tags
- Open phpMyAdmin and select the rfid_reg table.
- Insert the RFID UID manually.
- Set rfid_status to 1 for ACTIVE or 0 for INACTIVE.
- Scan again to verify the result.

Troubleshooting
- If the PHP page does not load, make sure Apache and MySQL are running.
- If ESP32 fails to connect, check your Wi-Fi credentials or local IP address.
- If RFID shows as NOT REGISTERED, add the UID manually to the rfid_reg table.
- If page does not auto-refresh, verify that the meta refresh tag is present in the PHP file.

Summary
- The ESP32 reads and sends RFID data to a PHP server through Wi-Fi.
- The PHP script stores and displays logs using MySQL.
- Each RFID scan alternates between active and inactive state.
- The dashboard now automatically refreshes every 1 second to show new logs in real time.
