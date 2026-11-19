
#include <SoftwareSerial.h>

// Nextion touchscreen
// D2 = RX, D3 = TX
SoftwareSerial nextion(2, 3);

// LoRa radio
// D10 = RX, D11 = TX
SoftwareSerial lora(10, 11);

String touchCommand = "";

// Response timeout
const unsigned long ACK_TIMEOUT = 8000;

// Additional time to receive Murphy feedback
const unsigned long FEEDBACK_TIMEOUT = 3000;

void setup()
{
  Serial.begin(9600);

  nextion.begin(9600);
  lora.begin(9600);

  nextion.listen();

  Serial.println("==============================");
  Serial.println(" WELL KILLER - HOME CONTROL");
  Serial.println("==============================");
  Serial.println("Ready for shutdown and feedback");
}

void loop()
{
  // ==========================================
  // LISTEN TO NEXTION TOUCHSCREEN
  // ==========================================

  nextion.listen();

  while (nextion.available())
  {
    char incoming = nextion.read();

    if (incoming >= 32 && incoming <= 126)
    {
      touchCommand += incoming;
    }

    delay(2);
  }

  // ==========================================
  // SHUTDOWN BUTTON PRESSED
  // ==========================================

  if (touchCommand.indexOf("WELL_STOP") >= 0)
  {
    Serial.println();
    Serial.println("SHUT DOWN button pressed.");

    // Display shutdown command status
    nextion.print("status.txt=\"SHUTDOWN SENT\"");
    sendNextionEnd();

    // Switch to LoRa
    lora.listen();

    // Send shutdown command
    lora.println("WELL_STOP");

    Serial.println("Sent: WELL_STOP");
    Serial.println("Waiting for well response...");

    touchCommand = "";

    // ========================================
    // WAIT FOR ACKNOWLEDGMENT
    // ========================================

    unsigned long startTime = millis();
    bool acknowledged = false;
    bool murphyTripped = false;

    while (millis() - startTime < ACK_TIMEOUT)
    {
      if (lora.available())
      {
        String response = lora.readStringUntil('\n');
        response.trim();

        Serial.print("Received: ");
        Serial.println(response);

        if (response == "ACK_RELAY_PULSED")
        {
          acknowledged = true;
          break;
        }

        if (response == "MURPHY_TRIPPED")
        {
          murphyTripped = true;
        }
      }
    }

    // ========================================
    // WAIT FOR MURPHY FEEDBACK
    // ========================================

    if (acknowledged && !murphyTripped)
    {
      Serial.println("Waiting for Murphy feedback...");

      unsigned long feedbackStart = millis();

      while (millis() - feedbackStart < FEEDBACK_TIMEOUT)
      {
        if (lora.available())
        {
          String response = lora.readStringUntil('\n');
          response.trim();

          Serial.print("Received: ");
          Serial.println(response);

          if (response == "MURPHY_TRIPPED")
          {
            murphyTripped = true;
            break;
          }
        }
      }
    }

    // ========================================
    // UPDATE NEXTION SCREEN
    // ========================================

    nextion.listen();

    if (acknowledged)
    {
      Serial.println("Well acknowledged shutdown.");

      if (murphyTripped)
      {
        nextion.print("status.txt=\"MURPHY TRIPPED\"");
        sendNextionEnd();

        Serial.println("Murphy feedback confirmed.");
      }
      else
      {
        nextion.print("status.txt=\"WELL STOPPED\"");
        sendNextionEnd();

        Serial.println("Shutdown acknowledged.");
        Serial.println("Murphy feedback not received.");
      }
    }
    else
    {
      Serial.println("ERROR: No response from well.");

      nextion.print("status.txt=\"NO RESPONSE\"");
      sendNextionEnd();
    }

    Serial.println();
  }

  // Prevent unexpected data accumulation
  if (touchCommand.length() > 40)
  {
    touchCommand = "";
  }
}

// ============================================
// NEXTION COMMAND TERMINATOR
// ============================================

void sendNextionEnd()
{
  nextion.write(0xFF);
  nextion.write(0xFF);
  nextion.write(0xFF);
}
