# Well Killer — Remote Irrigation Well Control (Student Innovation Project)

An Arduino Nano / UART LoRa bench prototype for sending remote irrigation-well shutdown **requests** from a Nextion touchscreen. The project is intended to reduce travel to remote irrigation engines during hazardous weather.

**Status (November 2026): bench prototype demonstrated.** The touchscreen, two-way LoRa link, three-second relay pulse, relay acknowledgment, and simulated optocoupler input have been tested. The Murphy 117 interface, actual engine shutdown, independent stopped/running feedback, field range, environmental protection, and safety validation remain **unverified**.

## Hardware tested

- Home: Arduino Nano, Nextion Enhanced 3.5-inch NX4832K035, DX-LR32-900T22D(T0) UART LoRa module.
- Well: Arduino Nano, matching UART LoRa module, 5 V active-HIGH relay board, 3–5 V input optocoupler module.
- Radio UART at 9600 baud; modules configured alike near 915 MHz. The implementation uses **direct LoRa**, not LoRaWAN; it does **not** implement encryption or command authentication.

## Wiring — bench prototype

| Side | Connection | Nano pin |
| --- | --- | --- |
| Home | Nextion TX → Nano RX | D2 |
| Home | Nextion RX ← Nano TX | D3 |
| Home & Well | LoRa TXD → Nano RX | D10 |
| Home & Well | LoRa RXD ← Nano TX | D11 |
| Well | Relay IN1 (active HIGH) | D4 |
| Well | Optocoupler OUT (active LOW) | D5 (`INPUT_PULLUP`) |

Use the correct regulated 5 V supply and shared ground on the **Nano/output** side. Keep the optocoupler input side electrically isolated. The optocoupler input is **3–5 V only**, not suitable for direct connection to a 12–13 V Murphy circuit. Verify radio supply requirements against the module datasheet.

## Firmware

- `firmware/Well_Killer_Home/Well_Killer_Home.ino`: receives Nextion `WELL_STOP` touch event, sends LoRa request, and updates the Nextion text object named `status`.
- `firmware/Well_Killer_Well/Well_Killer_Well.ino`: receives request, pulses relay D4 for three seconds, acknowledges the pulse, and reports D5 state transitions.

Arduino IDE: Board **Arduino Nano**, processor **ATmega328P (Old Bootloader)** on the tested boards; Serial Monitor **9600 baud**. Nextion `stopBtn` Touch Release Event: `prints "WELL_STOP",0`.

### Radio messages

| Message | Meaning |
| --- | --- |
| `WELL_STOP` | Request a relay pulse |
| `ACK_RELAY_PULSED` | Relay pulse completed; **not proof engine stopped** |
| `MURPHY_TRIPPED` | Optocoupler input became active; **bench-simulated** |
| `MURPHY_RESET` | Optocoupler input became inactive; **bench-simulated** |

The home screen uses the more cautious labels `MURPHY INPUT ACTIVE` / `MURPHY INPUT RESET` for these signals.

## Limitations and safety

**Educational prototype only. Not suitable for unattended or safety-critical engine shutdown.** Two `SoftwareSerial` ports on the home Nano cannot receive simultaneously, so radio messages can be lost. The plain-text command has no authentication, replay protection, retry scheme, or positive confirmation of physical engine shutdown. Timeout means **unknown outcome**, not proof of continued operation. Do not rely on this system for lightning protection or personnel safety.

The existing Murphy 117 / Swichgage / emergency-stop shutdown system must remain independently functional. Do not connect the 3–5 V optocoupler input directly to engine voltage or assume that the proposed relay connection is validated. An automotive-rated fused power interface, verified shutdown circuit ratings, isolated status interface, and controlled system-level testing are required before field use.

## Project site

- [SIP page](https://connorcqueen-lgtm.github.io/irrigation-well-control/sip.html)
- [Source repository](https://github.com/connorcqueen-lgtm/irrigation-well-control)

## Next milestones

1. Verify Murphy 117 shutdown-request and status circuits against the complete engine wiring.
2. Implement authenticated commands, message IDs, retries, and safer acknowledgment/state handling.
3. Confirm actual engine-running state independently and test safe failure behavior.
4. Validate fused automotive power, weatherproofing, radio range, and bill of materials.
