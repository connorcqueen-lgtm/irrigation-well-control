#include <SoftwareSerial.h>

// Well Killer bench prototype. Radio messages are NOT authenticated.
SoftwareSerial nextion(2, 3); // Nano RX=D2, TX=D3
SoftwareSerial lora(10, 11);  // Nano RX=D10, TX=D11
String touchCommand;
String radioLine;
bool waitingForAck = false;
unsigned long sentAt = 0;
const unsigned long ACK_TIMEOUT_MS = 8000;

void endNextion() {
  nextion.write(0xFF);
  nextion.write(0xFF);
  nextion.write(0xFF);
}

void showStatus(const char* label) {
  // Only fixed labels supplied by firmware are sent to the Nextion.
  nextion.print(F("status.txt=\""));
  nextion.print(label);
  nextion.print('"');
  endNextion();
}

void handleRadio(String msg) {
  msg.trim();
  Serial.print(F("Radio: "));
  Serial.println(msg);
  if (msg == "ACK_RELAY_PULSED") {
    waitingForAck = false;
    showStatus("RELAY ACTIVATED"); // Does not confirm engine stopped.
  } else if (msg == "MURPHY_TRIPPED") {
    showStatus("MURPHY INPUT ACTIVE"); // Bench-simulated status.
  } else if (msg == "MURPHY_RESET") {
    showStatus("MURPHY INPUT RESET");
  }
}

void setup() {
  Serial.begin(9600);
  nextion.begin(9600);
  lora.begin(9600);
  nextion.listen();
  Serial.println(F("Well Killer: bench home controller ready"));
}

void loop() {
  // SoftwareSerial can listen to only one UART at a time.
  // Short alternating windows reduce, but do not eliminate, lost messages.
  nextion.listen();
  unsigned long window = millis();
  while (millis() - window < 40) {
    while (nextion.available()) {
      char c = (char)nextion.read();
      if (c >= 32 && c <= 126) {
        if (touchCommand.length() < 40) touchCommand += c;
        else touchCommand = "";
      }
    }
  }

  if (touchCommand.indexOf("WELL_STOP") >= 0) {
    touchCommand = "";
    showStatus("SHUTDOWN SENT");
    lora.listen();
    lora.println(F("WELL_STOP"));
    waitingForAck = true;
    sentAt = millis();
    Serial.println(F("Stop command transmitted"));
  }

  lora.listen();
  window = millis();
  while (millis() - window < 180) {
    while (lora.available()) {
      char c = (char)lora.read();
      if (c == '\n') {
        handleRadio(radioLine);
        radioLine = "";
      } else if (c >= 32 && c <= 126) {
        if (radioLine.length() < 50) radioLine += c;
        else radioLine = "";
      }
    }
  }

  if (waitingForAck && millis() - sentAt >= ACK_TIMEOUT_MS) {
    waitingForAck = false;
    showStatus("NO RESPONSE"); // Outcome unknown: do not assume well is running.
  }
}
