#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ==== PIN DEFINITIONS ====
#define RELAY_PIN 12  // GPIO 12 for relay IN (ESP32 pin)

// ==== NETWORK & MQTT CONFIG ====
// const char* ssid = "2.4G-eWLU";      // Wi-Fi SSID
// const char* password = "UjKdmUrr";   // Wi-Fi password
const char* ssid = "Acerdano Wifi";
const char* password = "654753258951";
const char* mqtt_server = "192.168.100.5"; // Mosquitto broker IP
const int mqtt_port = 1883;          // MQTT port
const char* mqtt_topic = "RFID_LOGIN";  // PIT topic

WiFiClient espClient;
PubSubClient client(espClient);

// Track current state
bool relayState = false;

// ==== MQTT CALLBACK ====
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\n[MQTT] Message arrived on topic: ");
  Serial.println(topic);

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();
  Serial.print("[MQTT] Payload: ");
  Serial.println(message);

  // Handle "1" and "0"
  if (message == "1") {
    relayState = true;  // HIGH for "1"
  } else if (message == "0") {
    relayState = false;  // LOW for "0"
  } else {
    Serial.println("[WARN] Unknown message, ignoring.");
    return;
  }

  // Set relay pin (assuming active-high relay)
  digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);

  Serial.print("[ACTION] Relay set to ");
  Serial.println(relayState ? "ON (HIGH)" : "OFF (LOW)");
}

// ==== RECONNECT TO MQTT ====
void reconnect() {
  while (!client.connected()) {
    Serial.print("[MQTT] Attempting connection...");
    if (client.connect("ESP32_Relay_Subscriber")) {  // Updated client ID
      Serial.println("connected!");
      client.subscribe(mqtt_topic);
      Serial.print("[MQTT] Subscribed to topic: ");
      Serial.println(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// ==== SETUP ====
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32 Relay Subscriber Starting ===");

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // Initial OFF state

  // ==== Connect to Wi-Fi ====
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
  client.setCallback(callback);

  Serial.println("[SYSTEM] Setup complete. Waiting for messages...");
}

// ==== MAIN LOOP ====
void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}