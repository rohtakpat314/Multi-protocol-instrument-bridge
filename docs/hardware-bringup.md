# Hardware Bring-Up

This guide assumes a Raspberry Pi Pico or compatible RP2040 board.

## Pinout

| Protocol | Signal | Pico GPIO |
| --- | --- | --- |
| I2C0 | SDA | GP4 |
| I2C0 | SCL | GP5 |
| SPI0 | MISO | GP16 |
| SPI0 | CS | GP17 |
| SPI0 | SCK | GP18 |
| SPI0 | MOSI | GP19 |
| UART0 | TX | GP0 |
| UART0 | RX | GP1 |
| 1-Wire | DQ | GP22 |

## Electrical Notes

- The Pico is a 3.3 V device. Do not connect 5 V logic directly to GPIO pins.
- Use external pull-ups for I2C and 1-Wire during real bench use.
- Tie grounds together between the bridge and target instrument.
- Add level shifting when talking to 1.8 V, 2.5 V, or 5 V targets.
- Keep SPI wiring short at higher clock speeds.

## First Power-On Checklist

1. Flash the UF2 firmware.
2. Open the USB serial port from a terminal.
3. Run `help` and confirm the command list prints.
4. Run `status` and confirm default speeds.
5. With no external wiring attached, run `i2c scan`; it should usually return `OK` with no addresses.
6. Connect one known I2C board and run `i2c scan` again.
7. Validate SPI with a known flash chip or loopback fixture.
8. Validate UART with a TX-to-RX loopback.
9. Validate 1-Wire with a known DS18B20 or equivalent device.

## Suggested Bench Fixtures

| Protocol | Simple Fixture |
| --- | --- |
| I2C | EEPROM, temperature sensor, RTC, or IMU breakout. |
| SPI | JEDEC SPI flash chip. |
| UART | TX/RX loopback or USB UART dongle. |
| 1-Wire | DS18B20 temperature sensor with 4.7 kOhm pull-up. |

## Known Safe Defaults

| Bus | Default |
| --- | --- |
| I2C | 100 kHz |
| SPI | 1 MHz |
| UART | 115200 8N1 |
| 1-Wire | Standard speed timing |

## Troubleshooting

| Symptom | Likely Cause | Check |
| --- | --- | --- |
| No USB serial port | Firmware not flashed or USB cable is charge-only | Reflash UF2 and swap cable. |
| I2C scan finds nothing | Missing pull-ups, wrong voltage, swapped SDA/SCL | Probe idle lines; both should be high. |
| SPI returns all `0xFF` | MISO floating or target not selected | Check CS polarity and wiring. |
| UART reads nothing | TX/RX swapped incorrectly or baud mismatch | Cross TX to RX and confirm baud. |
| 1-Wire no presence | Missing pull-up or sensor power issue | Confirm DQ idles high and reset pulse is visible. |
