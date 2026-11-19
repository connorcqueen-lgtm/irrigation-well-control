# Well Killer KiCad schematics

Open the three `.kicad_sch` files in KiCad Schematic Editor (KiCad 8/9 recommended). These are editable, labeled *logical wiring schematics* with embedded custom symbols, not PCB designs or verified production wiring drawings.

## Sheets
- `Well_Killer_Home.kicad_sch` — home Nano, Nextion, SX1262 UART radio.
- `Well_Killer_Well.kicad_sch` — well Nano, SX1262 UART radio, four-channel relay module (two channels used), PC817 output.
- `Well_Killer_Motor_Feedback.kicad_sch` — AA motor supply, NC motor shutdown contact, USB power bank/HW-131, NO simulated Murphy contact, PC817 input.

## Verification required
- Confirm exact LoRa module supply voltage, interface logic levels, and pins from hardware datasheet. Do not assume generic SX1262 modules accept 5V.
- Confirm the four-channel relay board's VCC, input logic and COM/NC/NO terminal order.
- Confirm the PC817 module input polarity and rated input voltage.
- The Nano controller power wiring is based on earlier notes, not fully traced from photographs.
- KiCad net labels represent electrical connections; physical routing and connector placement remain to be verified.

## Scope
The 5V optocoupler signal is *simulated* Murphy feedback, not actual Murphy 117 or engine state. The low-voltage motor stop is not fail-safe under controller power loss.
