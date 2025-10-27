#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define SS_PIN   5
#define RST_PIN 22
#define LED_PIN  4

const char* preferred_ssid     = "Cloud Control Network";
const char* preferred_password = "ccv7network";
const char* fallback_ssid      = "Acerdano Wifi";
const char* fallback_password  = "654753258951";
const char* serverUrl = "http://192.168.100.5/rfid_system/insert_log.php";

MFRC522 rfid(SS_PIN, RST_PIN);

void connectWiFiAuto() {
  Serial.println("Scanning WiFi networks...");
  int n = WiFi.scanNetworks();
  bool foundPreferred = false;
  for (int i = 0; i < n; ++i) {
    String ss = WiFi.SSID(i);
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

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  SPI.begin(18, 19, 23, SS_PIN);
  rfid.PCD_Init();

  connectWiFiAuto();
  Serial.println("Ready to scan...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = uidToString(&rfid.uid);
  Serial.println("RFID UID: " + uid);

  digitalWrite(LED_PIN, HIGH);
  delay(150);
  digitalWrite(LED_PIN, LOW);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String postData = "rfid_data=" + uid;
    int httpCode = http.POST(postData);

    if (httpCode > 0) {
      String payload = http.getString();
      payload.trim();

      if (payload == "ACTIVE") {
        Serial.println("RFID STATUS: ACTIVE ✅");
      } else if (payload == "INACTIVE") {
        Serial.println("RFID STATUS: INACTIVE ❌");
      } else if (payload == "NOT_FOUND") {
        Serial.println("RFID NOT FOUND ❌");
      } else if (payload == "NO_DATA") {
        Serial.println("No RFID data received.");
      } else {
        Serial.println("Unexpected server response: " + payload);
      }
    } else {
      Serial.printf("HTTP POST failed, error: %d\n", httpCode);
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected - cannot POST");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(400);
}
