# ESP CAN Sniffer

A simple ESP32-based CAN bus sniffer for BMS traffic using an MCP2515 CAN controller.

## Overview

This project reads CAN frames from a BMS network and prints packet details over serial. It supports both standard and extended CAN IDs and scans common baud rates for MCP2515 initialization.

## Features

- ESP32 + MCP2515 CAN bus sniffer
- Auto-detects CAN baud rate from 500 kbps, 250 kbps, and 125 kbps
- Supports standard and extended CAN IDs
- Prints frame metadata, data bytes in HEX, and decimal representation
- Periodic summary of unique IDs seen

## Hardware

- ESP32 development board
- MCP2515 CAN controller module with TJA1050 or similar transceiver
- CAN bus from BMS

### MCP2515 wiring

This is an example wiring for the MCP2515 module to the ESP32:

| ESP32 pin | MCP2515 pin    | Notes |
|-----------|----------------|-------|
| GPIO18    | SCK            | SPI clock |
| GPIO19    | MISO  (SO)         | SPI data in |
| GPIO23    | MOSI  (SI)         | SPI data out |
| GPIO5     | CS             | SPI chip select |
| GPIO4     | INT            | MCP2515 interrupt |
| 5V        | VCC            | Use 5V supply as indicated |
| GND       | GND            | Common ground |

## Software

### Required library

The project depends on the `MCP_CAN_lib` library.

### Build and upload

1. Open the project in PlatformIO.
2. Select the `esp32dev` environment.
3. Build and upload to the ESP32.

### Serial monitor

Open the serial monitor at `115200` baud.

The output includes:

- connection status
- detected CAN frames
- frame type (STD/EXT)
- CAN ID
- DLC
- data bytes in HEX and DEC

## Notes

- The MCP2515 module is initialized with an 8 MHz crystal.
- If initialization fails, verify power, ground, and SPI wiring.
- The module will attempt 500 kbps, 250 kbps, then 125 kbps.

## License

This project is licensed under the MIT License.
