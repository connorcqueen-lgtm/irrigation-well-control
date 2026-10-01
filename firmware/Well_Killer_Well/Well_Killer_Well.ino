#include <SoftwareSerial.h>

// LoRa
// D10 = RX
// D11 = TX
SoftwareSerial lora(10, 11);

// Relay 1
const int RELAY_PIN = 4;

void setup()
{
  Serial.begin(9600);
  lora.begin(9600);

  pinMode(RELAY_PIN, OUTPUT);

  // Relay OFF at startup
  digitalWrite(RELAY_PIN, LOW);

  Serial.println("==============================");
  Serial.println(" WELL KILLER - WELL RECEIVER");
  Serial.println("==============================");
  Serial.println("Waiting for LoRa commands...");
}

void loop()
{
  if (lora.available())
  {
    String message = lora.readStringUntil('\n');
    message.trim();

    Serial.print("Received: ");
    Serial.println(message);

    if (message == "WELL_STOP")
    {
      Serial.println("*** SHUTDOWN COMMAND RECEIVED ***");

      // Activate shutdown relay
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay 1 ON");

      delay(3000);

      // Release relay
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay 1 OFF");

      // Send confirmation back to the home unit
      delay(250);

      lora.println("ACK_STOPPED");

      Serial.println("Sent: ACK_STOPPED");
      Serial.println("Waiting for next command...");
      Serial.println();
    }
  }
}