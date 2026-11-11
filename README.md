# Nesco VS-12 ESP32-S3 Controller

Replacement controller for a Nesco VS-12 vacuum sealer. The stock front-panel
control board (`HZC-VS5165-PCB1`) is replaced with an ESP32-S3-N16R8 DevKitC-1,
while the stock power board (`PCB20150605W2`) stays in place as the driver board.

This repository is just getting started. Firmware, the pin map, and wiring notes
will land here as each part of the conversion is brought up.

## Hardware

- Sealer: Nesco VS-12
- Stock front-panel board (being replaced): HZC-VS5165-PCB1
- Stock power / driver board (kept): PCB20150605W2
- Controller: ESP32-S3-N16R8 DevKitC-1

## Status

Scaffold only. The firmware is a PlatformIO / Arduino project targeting
`esp32-s3-devkitc-1`.

## License

MIT. See [LICENSE](LICENSE).
