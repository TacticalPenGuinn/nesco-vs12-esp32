# Nesco VS-12 ESP32-S3 Controller

Replacement controller for a Nesco VS-12 vacuum sealer. The stock front-panel
control board (`HZC-VS5165-PCB1`) is replaced with an ESP32-S3-N16R8 DevKitC-1,
while the stock power board (`PCB20150605W2`) stays in place as the driver
board.

The ESP32 drives the pump and release valve, watches the lid and vacuum
switches, and leaves hooks for the seal-bar heaters and NTC thermistor.

## Cycle

A typical run looks like this:

1. Type `m` in the serial monitor to start the pump (blocked if the lid is open).
2. Pump until the P-SW vacuum switch trips and holds for 250 ms.
3. Keep pumping for a 5 second top-off so the bag can stabilize.
4. Pump stops, then wait 3 seconds.
5. Open the EV release valve for 2 seconds, then close it.

The lid-open interlock and a 20 second pump timeout override everything else.

## Hardware

- Sealer: Nesco VS-12
- Stock front-panel board (being replaced): HZC-VS5165-PCB1
- Stock power / driver board (kept): PCB20150605W2
- Controller: ESP32-S3-N16R8 DevKitC-1

The front-panel connector is **CN9**. Its signals are:

| Signal | Role |
| --- | --- |
| +5V | Sealer logic supply (do not connect to the ESP32) |
| GND | Common ground |
| S-SW (SAFE) | Lid switch |
| HEAT1 | Seal-bar heater 1 (planned) |
| HEAT2 | Seal-bar heater 2 (planned) |
| MOTOR | Vacuum pump |
| EV | Release / evac solenoid |
| P-SW | Pressure / vacuum switch |
| NTC | Seal-bar thermistor (planned) |

## Pin map

| ESP32-S3 GPIO | Signal | Notes |
| --- | --- | --- |
| 4 | NTC | Planned |
| 5 | MOTOR | Active-high, driven directly from the 3.3V GPIO |
| 6 | EV | Active-high, driven directly from the 3.3V GPIO |
| 7 | HEAT1 | Planned |
| 15 | HEAT2 | Planned |
| 16 | P-SW | Switch to ground, `INPUT_PULLUP` |
| 17 | SAFE | Switch to ground, `INPUT_PULLUP` |
| 48 | Onboard RGB LED | Status: off / green / blue |

## Findings

- **MOTOR** is active-high and can be driven directly from a 3.3V GPIO. No level shifter was needed for the pump output.
- **SAFE** and **P-SW** are switches to ground. Read them with `INPUT_PULLUP`. Never feed more than 3.3V into an ESP32 GPIO; if a pull-up is added on the breadboard, tie it to the ESP's 3V3 pin, not the sealer's 5V rail.
- `lidClosed()` polarity depends on how the lid switch is wired. The current sketch treats **HIGH** as closed.
- The **K1** pressure sensor is a mechanical diaphragm vacuum switch with a factory trip-point adjustment screw, not an analog pressure transducer.
- Development is done over USB power, with only **GND** shared between the ESP32 and the sealer. The sealer's 5V is not connected to the ESP32.

## Usage

This is a [PlatformIO](https://platformio.org/) project.

```bash
pio run          # build
pio run -t upload
pio device monitor
```

The serial monitor runs at **115200** baud. Type `m` to toggle the pump.

Status LED:

- Off — lid open
- Green — lid closed, idle
- Blue — pump running

## Roadmap

- NTC thermistor calibration for seal-bar temperature
- Heater control, with an independent hardware thermal cutoff
- Wi-Fi status web page

## Safety

The sealer contains **mains voltage**. Unplug it before opening the case or probing the power board.

The seal-bar heaters should not be enabled in firmware alone. They need an independent hardware thermal cutoff, in addition to any software limit based on the NTC.

Do not connect the sealer's 5V rail to an ESP32 GPIO or to the ESP32's 5V/VBUS pin while the board is also powered from USB.

## Photos

Photos will live in `docs/images/`. Drop files in with these names when they are ready; this section is left as comments so the README does not render broken images.

<!-- Photo: docs/images/teardown.jpg — sealer teardown, front-panel board and CN9 -->
<!-- Photo: docs/images/power-board.jpg — stock power board (PCB20150605W2) -->
<!-- Photo: docs/images/breadboard.jpg — ESP32-S3 breadboard hookup -->
<!-- Photo: docs/images/k1-vacuum-switch.jpg — K1 mechanical diaphragm vacuum switch -->

## License

MIT. See [LICENSE](LICENSE).
