# Hardware Notes

This folder is reserved for board-level design artifacts: schematics, pin tables, fixture wiring, and adapter board notes.

The initial firmware targets a stock Raspberry Pi Pico with no custom PCB. A useful next hardware milestone would be a small shield with:

- I2C pull-ups selectable by jumper.
- 3.3 V target power switch with current limiting.
- Level shifting for 1.8 V and 5 V targets.
- Multiple SPI chip-select headers.
- UART header with TX/RX labels and ground reference.
- ESD protection on external-facing pins.

## Default Connector Proposal

| Pin | Signal |
| --- | --- |
| 1 | GND |
| 2 | 3V3 reference |
| 3 | I2C SDA |
| 4 | I2C SCL |
| 5 | SPI MOSI |
| 6 | SPI MISO |
| 7 | SPI SCK |
| 8 | SPI CS |
| 9 | UART TX |
| 10 | UART RX |
| 11 | 1-Wire DQ |
| 12 | Fixture GPIO / future use |
