#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- Pins for RC522 and LED ---
#define SS_PIN   5
#define RST_PIN 22
#define LED_PIN  4

// WiFi preferred + fallback (edit fallback) ---
const char* preferred_ssid     = "Cloud Control Network";
const char* preferred_password = "ccv7network";

// Transfer fallback to PHP UI for better usability
// or
// Change fallback_ssid and fallback_password to preffered network if Cloud Control Network is not available

const char* fallback_ssid      = "STUDENT-CONNECT";     // <-- change
const char* fallback_password  = "IloveUSTP!";     // <-- change

// Server endpoint (Change to your server IP)
const char* serverUrl = "http://192.168.100.5/rfid_system/insert_log.php"; // <-- change to your PC IP

MFRC522 rfid(SS_PIN, RST_PIN);

void connectWiFiAuto() {
  Serial.println("Scanning WiFi networks...");
  int n = WiFi.scanNetworks();
  bool foundPreferred = false;
  for (int i = 0; i < n; ++i) {
    String ss = WiFi.SSID(i);
    Serial.println("Found: " + ss);
    if (ss.equals(String(preferred_ssid))) foundPreferred = true;
  }
  if (foundPreferred) {
    Serial.println("Connecting to preferred (Cloud Control Network)...");
    WiFi.begin(preferred_ssid, preferred_password);
  } else {
    Serial.println("Preferred not found — connecting to fallback...");
    WiFi.begin(fallback_ssid, fallback_password);
  }
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 30000) {
    Serial.print(".");
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed.");
  }
}

String uidToString(MFRC522::Uid *u) {
  String s = "";
  for (byte i = 0; i < u->size; i++) {
    if (u->uidByte[i] < 0x10) s += "0";
    s += String(u->uidByte[i], HEX);
  }
  s.toUpperCase();
  return s;
}

void setup(){
  Serial.begin(115200);
  delay(100);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  SPI.begin(18, 19, 23, SS_PIN);
  rfid.PCD_Init();
  Serial.println("RFID reader initialised.");

  connectWiFiAuto();
  Serial.println("Ready to scan...");
}

void loop(){
  // Non-blocking check for card
  if (!rfid.PICC_IsNewCardPresent()) { delay(50); return; }
  if (!rfid.PICC_ReadCardSerial()) { delay(50); return; }

  String uid = uidToString(&rfid.uid);
  Serial.println("RFID UID: " + uid);

  // ESP32 Blue LED
  digitalWrite(LED_PIN, HIGH);
  delay(150);
  digitalWrite(LED_PIN, LOW);

  // Send UID to server via HTTP POST
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Send uppercase trimmed UID only; let server decide status
    String postData = "rfid_data=" + uid;
    int httpCode = http.POST(postData);

    if (httpCode > 0) {
      String payload = http.getString();
      payload.trim();
      // Serial.print("HTTP code: "); Serial.println(httpCode);
      // Serial.print("Server returned: "); Serial.println(payload);

      if (payload == "VALID") {
        Serial.println("RFID STATUS: ACTIVE ✅");
        Serial.println("================================");
      } else if (payload == "INACTIVE") {
        Serial.println("RFID STATUS: INACTIVE ❌");
        Serial.println("================================");
      } else if (payload == "NOT_FOUND") {
        Serial.println("RFID NOT FOUND ❌");
        Serial.println("================================");
      } else if (payload == "NO_DATA") {
        Serial.println("Server: No data received.");
        Serial.println("================================");
      } else {
        Serial.println("Unexpected server response: " + payload);
        Serial.println("================================");
      }
    } else {
      Serial.print("HTTP POST failed, error: ");
      Serial.println(httpCode);
      Serial.println("================================");
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected - cannot POST");
    Serial.println("================================");
  }

  // Release card and small debounce
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(400); // avoid duplicate reads
}
