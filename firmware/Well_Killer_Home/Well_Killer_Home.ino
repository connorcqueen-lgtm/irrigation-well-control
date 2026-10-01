#include <SoftwareSerial.h>

// Nextion touchscreen
// D2 = RX, D3 = TX
SoftwareSerial nextion(2, 3);

// LoRa radio
// D10 = RX, D11 = TX
SoftwareSerial lora(10, 11);

String touchCommand = "";

void setup()
{
  Serial.begin(9600);

  nextion.begin(9600);
  lora.begin(9600);

  nextion.listen();

  Serial.println("==============================");
  Serial.println(" WELL KILLER - HOME CONTROL");
  Serial.println("==============================");
  Serial.println("Waiting for touchscreen...");
}

void loop()
{
  // ==================================================
  // LISTEN TO NEXTION
  // ==================================================

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

  // ==================================================
  // SHUTDOWN BUTTON PRESSED
  // ==================================================

  if (touchCommand.indexOf("WELL_STOP") >= 0)
  {
    Serial.println();
    Serial.println("SHUT DOWN button pressed.");

    // Tell the touchscreen the command was sent.
    nextion.print("status.txt=\"SHUTDOWN SENT\"");
    sendNextionEnd();

    // Switch to LoRa.
    lora.listen();

    // Send shutdown command.
    lora.println("WELL_STOP");

    Serial.println("Sent: WELL_STOP");
    Serial.println("Waiting for well response...");

    touchCommand = "";

    // ==================================================
    // WAIT FOR ACKNOWLEDGMENT
    // ==================================================

    unsigned long startTime = millis();
    bool acknowledged = false;

    // Wait up to 8 seconds.
    while (millis() - startTime < 8000)
    {
      if (lora.available())
      {
        String response = lora.readStringUntil('\n');
        response.trim();

        Serial.print("Received: ");
        Serial.println(response);

        if (response == "ACK_STOPPED")
        {
          acknowledged = true;
          break;
        }
      }
    }

    // ==================================================
    // UPDATE NEXTION
    // ==================================================

    nextion.listen();

    if (acknowledged)
    {
      Serial.println("Well acknowledged shutdown.");

      nextion.print("status.txt=\"WELL STOPPED\"");
      sendNextionEnd();
    }
    else
    {
      Serial.println("ERROR: No response from well.");

      nextion.print("status.txt=\"NO RESPONSE\"");
      sendNextionEnd();
    }

    Serial.println();
  }

  // Clear unexpected data.
  if (touchCommand.length() > 40)
  {
    touchCommand = "";
  }
}


// ======================================================
// NEXTION COMMAND TERMINATOR
//
// Every command sent TO a Nextion must end with three
// bytes of 0xFF.
// ======================================================

void sendNextionEnd()
{
  nextion.write(0xFF);
  nextion.write(0xFF);
  nextion.write(0xFF);
}