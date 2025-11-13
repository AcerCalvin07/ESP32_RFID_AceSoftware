#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define RELAY_PIN 12

// ==== Wi-Fi Credentials ====
const char* ssid = "Acerdano Wifi";
const char* password = "654753258951";

// const char* ssid = "Cloud Control Network";
// const char* password = "ccv7network";

const char* mqtt_server = "192.168.100.5";
const int mqtt_port = 1883;
const char* mqtt_topic = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient client(espClient);
bool relayState = false;

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();

  Serial.print("[STATUS] ");
  Serial.println(message);

  if (message == "1") {
    relayState = true;
  } else if (message == "0") {
    relayState = false;
  }

  digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);
  Serial.print("[ACTION] ");
  Serial.println(relayState ? "ON" : "OFF");
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32_Relay_Subscriber")) {
      client.subscribe(mqtt_topic);
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // ==== Wi-Fi Connection ====
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting to ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WIFI] Connected!");
  Serial.print("Waiting for Scan...");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
