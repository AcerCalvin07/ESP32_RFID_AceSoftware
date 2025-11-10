#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>

// ==== RFID PIN SETUP ====
#define SS_PIN 5
#define RST_PIN 22
MFRC522 rfid(SS_PIN, RST_PIN);

// ==== STATUS LED ====
#define LED_PIN 4

// ==== Wi-Fi Credentials ====
// const char* ssid = "2.4G-eWLU";
// const char* password = "UjKdmUrr";

const char* ssid = "Acerdano Wifi";
const char* password = "654753258951";

// ==== PHP Server Endpoint ====  
const char* serverUrl = "http://192.168.100.5/rfid_system/insert_log.php";

// ==== MQTT Broker ====
const char* mqtt_server = "192.168.100.5";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";  // PIT topic

// ==== MQTT Setup ====
WiFiClient espClient;
PubSubClient client(espClient);

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("[MQTT] Attempting connection...");
    if (client.connect("ESP32_RFID_Publisher")) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("[INIT] RFID Reader Initialized.");

  // ==== Wi-Fi Connection ====
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WIFI] Connected!");
  Serial.print("[WIFI] IP Address: ");
  Serial.println(WiFi.localIP());

  // ==== MQTT Setup ====
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  // ==== Check for new RFID Tag ====
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  // ==== Get RFID Tag UID ====
  String rfidTag = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    rfidTag += String(rfid.uid.uidByte[i], HEX);
  }
  rfidTag.toUpperCase();

  Serial.println();
  Serial.print("[SCAN] RFID Tag detected: ");
  Serial.println(rfidTag);

  // ==== Send RFID Data to PHP ====
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(serverUrl) + "?rfid_data=" + rfidTag;

    Serial.print("[HTTP] Sending request: ");
    Serial.println(url);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.print("[HTTP] Response Code: ");
      Serial.println(httpCode);

      String payload = http.getString();
      Serial.print("[HTTP] Server Response: ");
      Serial.println(payload);

      // ==== Refactored Logic: Publish 1 only for ACTIVE (registered + active), 0 for others ====
      if (payload.indexOf("\"status\":\"ACTIVE\"") != -1) {
        client.publish(mqtt_topic, "1");  // Gate opens
        digitalWrite(LED_PIN, HIGH);
        Serial.println("[MQTT] Published '1' → LED ON (Registered & Active).");
      } else if (payload.indexOf("\"status\":\"INACTIVE\"") != -1 || payload.indexOf("\"status\":\"NOT_REGISTERED\"") != -1) {
        client.publish(mqtt_topic, "0");  // Gate stays closed
        digitalWrite(LED_PIN, LOW);
        Serial.println("[MQTT] Published '0' → LED OFF (Inactive or Unregistered).");
      } else {
        Serial.println("[ERROR] Unrecognized response.");
      }

    } else {
      Serial.print("[HTTP] Request failed! Code: ");
      Serial.println(httpCode);
    }

    http.end();
  } else {
    Serial.println("[WIFI] Not connected! Skipping HTTP request.");
  }

  // ==== Cleanup RFID Communication ====
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1500);
}