#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>

// ==== RFID Pins ====
#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

// ==== LED Indicator ====
#define LED_PIN 4

// ==== Wi-Fi ====
const char* ssid = "Cloud Control Network";
const char* password = "ccv7network";

// ==== Server + MQTT ====
const char* serverUrl  = "http://10.222.98.120/rfid_system/insert_log.php";
const char* mqtt_server = "10.222.98.120";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient client(espClient);

// ==== MQTT Reconnect Timers ====
unsigned long lastMQTTAttempt = 0;
unsigned long mqttRetryInterval = 3000;

void reconnectMQTT() {
  if (client.connected()) return;

  unsigned long now = millis();
  if (now - lastMQTTAttempt < mqttRetryInterval) return;

  lastMQTTAttempt = now;

  Serial.println("[MQTT] Attempting reconnect...");
  client.connect("ESP32_RFID_Publisher");
  Serial.println("[MQTT] Reconnected!");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("[RFID] Ready.");

  // ==== Wi-Fi ====
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }

  Serial.println("\n[WIFI] Connected!");
  Serial.print("[WIFI] IP: ");
  Serial.println(WiFi.localIP());

  // ==== MQTT ====
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Non-blocking reconnect
  if (!client.connected()) reconnectMQTT();
  else client.loop();

  // RFID read condition
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  // Extract UID
  String rfidTag = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    rfidTag += String(rfid.uid.uidByte[i], HEX);
  }
  rfidTag.toUpperCase();

  Serial.print("\n[RFID] ");
  Serial.println(rfidTag);

  // Send to PHP
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverUrl) + "?rfid_data=" + rfidTag;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.print("[HTTP] Response: ");
      Serial.println(payload);

      // Process response
      if (payload.indexOf("\"status\":\"ACTIVE\"") != -1) {
        Serial.println("[STATUS] ACTIVE → Sending 1");
        client.publish(mqtt_topic, "1");
        digitalWrite(LED_PIN, HIGH);
      }
      else {
        Serial.println("[STATUS] INACTIVE/NOT FOUOND → Sending 0");
        client.publish(mqtt_topic, "0");
        digitalWrite(LED_PIN, LOW);
      }
    }

    http.end();
  }

  // Reset reader
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1500); 
}
