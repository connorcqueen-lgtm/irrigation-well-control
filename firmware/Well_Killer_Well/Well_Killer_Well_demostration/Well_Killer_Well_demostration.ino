
#include <SoftwareSerial.h>

// LoRa communication
// RX = D10, TX = D11
SoftwareSerial lora(10, 11);

// Pin assignments
const int RELAY1_PIN = 4;  // Motor shutdown
const int RELAY2_PIN = 8;  // Simulated Murphy signal
const int OPTO_PIN = 5;    // Optocoupler feedback

// Track optocoupler state
bool lastMurphyState = false;

// Track shutdown state
bool wellStopped = false;

void setup() {
  Serial.begin(9600);
  lora.begin(9600);

  // Both relay channels are assumed active HIGH
  // LOW = relay OFF
  // HIGH = relay ON
  digitalWrite(RELAY1_PIN, LOW);
  digitalWrite(RELAY2_PIN, LOW);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  // Optocoupler output is active LOW
  pinMode(OPTO_PIN, INPUT_PULLUP);

  // Read initial optocoupler state
  lastMurphyState = (digitalRead(OPTO_PIN) == LOW);

  Serial.println("WELL KILLER - WELL RECEIVER");
  Serial.println("LoRa and Murphy feedback ready");
  Serial.println("Two-relay demonstration ready");
}

void loop() {

  // -----------------------------------
  // 1. Monitor optocoupler feedback
  // -----------------------------------

  bool murphyState = (digitalRead(OPTO_PIN) == LOW);

  // Only send feedback when the state changes
  if (murphyState != lastMurphyState) {

    delay(30);  // Simple debounce

    murphyState = (digitalRead(OPTO_PIN) == LOW);

    if (murphyState != lastMurphyState) {

      lastMurphyState = murphyState;

      if (murphyState) {
        Serial.println("Murphy input ACTIVE");
        lora.println("MURPHY_TRIPPED");
        Serial.println("Sent: MURPHY_TRIPPED");
      }
      else {
        Serial.println("Murphy input INACTIVE");
        lora.println("MURPHY_RESET");
        Serial.println("Sent: MURPHY_RESET");
      }
    }
  }

  // -----------------------------------
  // 2. Receive LoRa commands
  // -----------------------------------

  if (lora.available()) {

    String message = lora.readStringUntil('\n');
    message.trim();

    Serial.print("Received: ");
    Serial.println(message);

    // -----------------------------------
    // STOP COMMAND
    // -----------------------------------

    if (message == "WELL_STOP") {

      Serial.println("Shutdown command received");

      // Relay 1 opens COM1-NC1
      // This disconnects the model motor
      digitalWrite(RELAY1_PIN, HIGH);

      // Relay 2 closes COM2-NO2
      // This applies 5V to the optocoupler
      digitalWrite(RELAY2_PIN, HIGH);

      wellStopped = true;

      Serial.println("Relay 1 ON - motor stopped");
      Serial.println("Relay 2 ON - simulated Murphy signal");

      // Confirm command was processed
      // Give the home controller time to prepare for the response
     delay(500);
     lora.println("ACK_RELAY_PULSED");
     Serial.println("Sent: ACK_RELAY_PULSED");
    }

    // -----------------------------------
    // RESET COMMAND
    // -----------------------------------

    else if (message == "WELL_RESET") {

      Serial.println("Reset command received");

      // Release Relay 1
      // Motor power is restored
      digitalWrite(RELAY1_PIN, LOW);

      // Release Relay 2
      // Simulated Murphy signal turns off
      digitalWrite(RELAY2_PIN, LOW);

      wellStopped = false;

      Serial.println("Relay 1 OFF - motor power restored");
      Serial.println("Relay 2 OFF - Murphy signal removed");

      // Confirm reset command
      lora.println("ACK_WELL_RESET");
      Serial.println("Sent: ACK_WELL_RESET");
    }
  }
}

