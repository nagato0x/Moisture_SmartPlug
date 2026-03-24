#include <SoftwareSerial.h>

// ====== PINS ======
const int moisturePin = A0;
const int relayPin    = 5;

// 🔥 LEDs
const int ledON  = 13;   // Relay ON indicator
const int ledOFF = 12;   // Relay OFF indicator

// 🔥 Bluetooth
SoftwareSerial BT(7, 10);

// ====== THRESHOLD ======
const int THRESHOLD = 700;

bool relayState = false;
bool lastRelayState = false;

unsigned long lastReadMs = 0;
const unsigned long READ_INTERVAL_MS = 1000;

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(ledON, OUTPUT);
  pinMode(ledOFF, OUTPUT);

  digitalWrite(relayPin, LOW);
  digitalWrite(ledON, LOW);
  digitalWrite(ledOFF, HIGH); // start as OFF

  Serial.begin(9600);
  BT.begin(9600);

  delay(2000);
  Serial.println("System start (Bluetooth mode)...");
}

void loop() {

  // ====== SERIAL BRIDGE ======
  while (Serial.available()) {
    BT.write(Serial.read());
  }
  while (BT.available()) {
    Serial.write(BT.read());
  }

  // ====== SENSOR + RELAY ======
  if (millis() - lastReadMs >= READ_INTERVAL_MS) {
    lastReadMs = millis();

    int moistureValue = analogRead(moisturePin);

    Serial.print("\nMoisture ADC Value: ");
    Serial.println(moistureValue);

    bool isDry = (moistureValue >= THRESHOLD);

    lastRelayState = relayState;
    relayState = isDry;

    // ⚠️ Change to LOW/HIGH if your relay is active LOW
    digitalWrite(relayPin, relayState ? HIGH : LOW);

    Serial.print("Relay: ");
    Serial.println(relayState ? "ON (DRY)" : "OFF (WET/OK)");

    // ====== LED INDICATION ======
    if (relayState) {
      digitalWrite(ledON, HIGH);
      digitalWrite(ledOFF, LOW);
    } else {
      digitalWrite(ledON, LOW);
      digitalWrite(ledOFF, HIGH);
    }

    // ====== SEND VIA BLUETOOTH ON CHANGE ======
    if (relayState != lastRelayState) {
      Serial.println("Sending via Bluetooth...");
      if (relayState) {
        BT.println("RELAY_ON");
      } else {
        BT.println("RELAY_OFF");
      }
    }
  }
}