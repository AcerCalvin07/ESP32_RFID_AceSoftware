#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>

#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

#define LED_PIN 4

// ==== Wi-Fi Credentials ====
const char* ssid = "Acerdano Wifi";
const char* password = "654753258951";

// const char* ssid = "Cloud Control Network";
// const char* password = "ccv7network";

const char* serverUrl = "http://192.168.100.5/rfid_system/insert_log.php";
const char* mqtt_server = "192.168.100.5";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient client(espClient);

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32_RFID_Publisher")) break;
    delay(2000);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  SPI.begin();
  rfid.PCD_Init();

  // ==== Wi-Fi Connection ====
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WIFI] Connected!");
  Serial.print("Ready to scan RFID...");

  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String rfidTag = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    rfidTag += String(rfid.uid.uidByte[i], HEX);
  }
  rfidTag.toUpperCase();

  Serial.println();
  Serial.print("[RFID ID] ");
  Serial.println(rfidTag);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverUrl) + "?rfid_data=" + rfidTag;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();

      if (payload.indexOf("\"status\":\"ACTIVE\"") != -1) {
        Serial.println("[STATUS] 1");
        client.publish(mqtt_topic, "1");
        digitalWrite(LED_PIN, HIGH);
      } 
      else if (payload.indexOf("\"status\":\"INACTIVE\"") != -1) {
        Serial.println("[STATUS] 0");
        client.publish(mqtt_topic, "0");
        digitalWrite(LED_PIN, LOW);
      } 
      else if (payload.indexOf("\"status\":\"NOT_REGISTERED\"") != -1) {
        Serial.println("[STATUS] RFID NOT FOUND");
        client.publish(mqtt_topic, "0");
        digitalWrite(LED_PIN, LOW);
      }
    }
    http.end();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1500);
}
