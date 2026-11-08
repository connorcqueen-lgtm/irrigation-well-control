#include <SoftwareSerial.h>

// Well Killer bench prototype. Not validated for unattended engine shutdown.
SoftwareSerial lora(10, 11); // Nano RX=D10, TX=D11
const byte RELAY_PIN = 4;   // Tested active-HIGH relay board
const byte OPTO_PIN = 5;    // Tested active-LOW isolated input
const unsigned long PULSE_MS = 3000;
const unsigned long DEBOUNCE_MS = 30;

bool relayActive = false;
unsigned long relayStarted = 0;
bool stableInput = false;
bool lastRawInput = false;
unsigned long inputChangedAt = 0;
String incoming;

void setup() {
  Serial.begin(9600);
  lora.begin(9600);
  lora.setTimeout(50);
  digitalWrite(RELAY_PIN, LOW); // Set inactive level before output mode
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(OPTO_PIN, INPUT_PULLUP);
  stableInput = (digitalRead(OPTO_PIN) == LOW);
  lastRawInput = stableInput;
  inputChangedAt = millis();
  Serial.println(F("Well Killer: bench receiver ready"));
}

void loop() {
  const unsigned long now = millis();

  // Complete pulse without blocking input monitoring.
  if (relayActive && now - relayStarted >= PULSE_MS) {
    digitalWrite(RELAY_PIN, LOW);
    relayActive = false;
    lora.println(F("ACK_RELAY_PULSED"));
    Serial.println(F("Relay released; pulse acknowledged (not engine status)"));
  }

  const bool raw = (digitalRead(OPTO_PIN) == LOW);
  if (raw != lastRawInput) {
    lastRawInput = raw;
    inputChangedAt = now;
  }
  if (raw != stableInput && now - inputChangedAt >= DEBOUNCE_MS) {
    stableInput = raw;
    // This is a SIMULATED input in bench testing, NOT verified engine RPM.
    lora.println(stableInput ? F("MURPHY_TRIPPED") : F("MURPHY_RESET"));
    Serial.println(stableInput ? F("Simulated trip input ACTIVE") : F("Simulated trip input INACTIVE"));
  }

  while (lora.available()) {
    char c = (char)lora.read();
    if (c == '\n') {
      incoming.trim();
      if (incoming == "WELL_STOP") {
        if (!relayActive) {
          relayActive = true;
          relayStarted = millis();
          digitalWrite(RELAY_PIN, HIGH);
          Serial.println(F("Relay pulse started"));
        } else {
          Serial.println(F("Duplicate stop command during pulse ignored"));
        }
      }
      incoming = "";
    } else if (c >= 32 && c <= 126) {
      if (incoming.length() < 40) incoming += c;
      else incoming = "";
    }
  }
}
