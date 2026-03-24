#include <SoftwareSerial.h>

// ====== PINS (same as you requested) ======
const int moisturePin = A0;   // FC-37 / YT83 AO
const int relayPin    = 5;    // Relay IN

// SIM900A: UNO D10=RX (from GSM TX), D11=TX (to GSM RX via divider)
SoftwareSerial gsm(10, 11);

// ====== THRESHOLD ======
const int THRESHOLD = 700;   // <-- Set analogRead threshold to 700

String phoneNumber = "+94766202660";  // change this

bool relayState = false;
bool lastRelayState = false;

unsigned long lastReadMs = 0;
const unsigned long READ_INTERVAL_MS = 1000;

void sendAT(const String &cmd, unsigned long waitMs = 600) {
  gsm.println(cmd);
  delay(waitMs);
}

void sendSMS(const String &number, const String &message) {
  gsm.println("AT");
  delay(500);
  gsm.println("AT+CMGF=1"); // SMS text mode
  delay(500);

  gsm.print("AT+CMGS=\"");
  gsm.print(number);
  gsm.println("\"");
  delay(500);

  gsm.print(message);
  gsm.write(26); // CTRL+Z
  delay(5000);
}

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // OFF at start (invert if your relay is active-LOW)

  Serial.begin(9600);  // USB Serial Monitor
  gsm.begin(9600);     // SIM900A

  delay(2000);
  Serial.println("System start...");
  Serial.println("Type AT commands in Serial Monitor; they will go to SIM900A.");
  Serial.println("Moisture threshold = 700 (>=700 means DRY -> Relay ON)");
}

void loop() {
  // ====== SERIAL BRIDGE (Serial Monitor <-> SIM900A) ======
  while (Serial.available()) {
    gsm.write(Serial.read());
  }
  while (gsm.available()) {
    Serial.write(gsm.read());
  }

  // ====== READ SENSOR + CONTROL RELAY ======
  if (millis() - lastReadMs >= READ_INTERVAL_MS) {
    lastReadMs = millis();

    int moistureValue = analogRead(moisturePin);  // <-- your required line

    Serial.print("\nMoisture ADC Value: ");
    Serial.println(moistureValue);

    // If moistureValue >= 700 => DRY => Relay ON
    bool isDry = (moistureValue >= THRESHOLD);

    lastRelayState = relayState;
    relayState = isDry;

    // Relay drive (if your relay is active LOW, invert this line)
    digitalWrite(relayPin, relayState ? HIGH : LOW);

    Serial.print("Relay: ");
    Serial.println(relayState ? "ON (DRY)" : "OFF (WET/OK)");

    // ====== SEND SMS ONLY WHEN STATE CHANGES ======
    if (relayState != lastRelayState) {
      String msg = relayState
        ? ("DRY detected! Relay ON. ADC=" + String(moistureValue))
        : ("Soil OK/WET. Relay OFF. ADC=" + String(moistureValue));

      Serial.println("Sending SMS...");
      sendSMS(phoneNumber, msg);
      Serial.println("SMS sent command (watch GSM responses above).");
    }
  }
}
