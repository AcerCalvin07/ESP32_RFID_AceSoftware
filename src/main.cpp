// --- Libraries ---
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// --- Pin Definitions ---
#define SS_PIN   5   // Slave Select pin for RFID
#define RST_PIN 22   // Reset pin for RFID
#define LED_PIN  4   // LED indicator pin

// --- RFID Object ---
MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  delay(100);

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Start SPI communication with custom ESP32 pins
  SPI.begin(18, 19, 23, SS_PIN);

  // Initialize RFID reader
  rfid.PCD_Init();

  // Print startup message
  Serial.println("\n=== ESP32 MFRC522 RFID Reader ===");
  Serial.println("Ready to scan...");
}

// --- Convert UID bytes into a readable HEX string ---
String uidToString(MFRC522::Uid *uid) {
  String s = "";
  for (byte i = 0; i < uid->size; i++) {
    if (uid->uidByte[i] < 0x10) s += "0";     
    s += String(uid->uidByte[i], HEX);        
  }
  s.toUpperCase();                            
  return s;
}

void loop() {
  // --- Check if a new RFID card is present ---
  if (!rfid.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }

  // --- Try to read the card's UID ---
  if (!rfid.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // --- Convert UID to string and print it ---
  String uid = uidToString(&rfid.uid);
  Serial.print("RFID UID: ");
  Serial.println(uid);

  // --- Blink LED to indicate successful read ---
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);

  // --- Stop communication with the card ---
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  // --- Small delay before next scan ---
  delay(300);
}
