#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define RELAY_PIN 12  // GPIO12

// ==== Wi-Fi ====
const char* ssid     = "Cloud Control Network";
const char* password = "ccv7network";

// ==== MQTT ====
const char* mqtt_server = "10.222.98.120";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "RFID_LOGIN";

WiFiClient espClient;
PubSubClient client(espClient);

// Track state
bool relayState = false;

void callback(char* topic, byte* payload, unsigned int length) {
  String message;

  for (unsigned int i = 0; i < length; i++)
    message += (char)payload[i];

  message.trim();

  Serial.print("");
  Serial.print("[MQTT] Received: ");
  Serial.println(message);

  if (message == "1") {
    relayState = true;
  } 
  else if (message == "0") {
    relayState = false;
  }

  // Relay is active LOW
  digitalWrite(RELAY_PIN, relayState ? LOW : HIGH);

  Serial.print("[RELAY] ");
  Serial.println(relayState ? "ON" : "OFF");
}

unsigned long lastMQTTAttempt = 0;
unsigned long mqttRetry = 3000;

void reconnect() {
  if (client.connected()) return;

  unsigned long now = millis();
  if (now - lastMQTTAttempt < mqttRetry) return;

  lastMQTTAttempt = now;

  Serial.println("[MQTT] Reconnecting...");
  if (client.connect("ESP32_Relay_Subscriber")) {
    client.subscribe(mqtt_topic);
    Serial.println("[MQTT] Reconnected & subscribed");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); 

  // Connect Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("[WIFI] Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }

  Serial.println("\n[WIFI] Connected!");
  Serial.print("[WIFI] IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  else client.loop();
}
