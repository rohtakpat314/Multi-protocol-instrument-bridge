# USB Bridge

USB-to-everything bench adapter for instrument bring-up and test automation.

A USB CDC command adapter for I2C, SPI, UART, and bit-banged 1-Wire targets. The host PC sees a virtual serial port; the board accepts readable text commands and executes bus transactions on the target side.

This is the kind of utility a hardware validation or internal test team builds when they need something smaller and more hackable than a Bus Pirate, FTDI MPSSE adapter, or dedicated lab instrument.

## Highlights

- USB CDC virtual serial interface.
- Text command parser written in portable C.
- I2C scan, read, write, and repeated-start write-read.
- SPI full-duplex transfers with chip-select control.
- UART read/write with configurable baud rate.
- Standard-speed bit-banged 1-Wire reset, read, and write.
- Host-side parser tests using fake bus backends.
- Hardware backend isolated behind `include/bridge/bus.h`.
- Formal LaTeX technical manual and Markdown engineering docs.

## Repository Layout

```text
.
|-- CMakeLists.txt
|-- README.md
|-- include/bridge/
|   |-- bus.h
|   |-- command.h
|   `-- line_editor.h
|-- src/
|   |-- bus_pico.c
|   |-- command.c
|   |-- line_editor.c
|   `-- main.c
|-- tests/
|   `-- parser_tests.c
|-- docs/
|   |-- architecture.md
|   |-- command-reference.md
|   |-- hardware-bringup.md
|   |-- testing.md
|   `-- latex/
|       `-- manual.tex
|-- hardware/
|   `-- README.md
|-- examples/
|   `-- README.md
`-- scripts/
    `-- README.md
```

## Hardware Target

The current firmware targets Raspberry Pi Pico / RP2040 using the Pico SDK.

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

The Pico is a 3.3 V device. Use external level shifting for non-3.3 V targets. I2C and 1-Wire need pull-ups; internal pull-ups are enabled for convenience, but external pull-ups are recommended for real bench use.

## Command Examples

Commands are newline-terminated text sent over the USB virtual serial port.

```text
help
status
i2c scan
i2c speed 400000
i2c read 0x68 6
i2c write 0x1A 0xFF 0x00
i2c wr 0x68 2 0x75
spi speed 1000000
spi xfer 0x9F 0x00 0x00 0x00
uart speed 115200
uart write 0x55 0x0D 0x0A
uart read 16 250
ow reset
ow write 0x33
ow read 8
```

Numbers can be decimal, hex (`0x1A`), octal (`032`), or binary (`0b11010`). Replies start with `OK`, `ERR`, or `TIMEOUT`.

## Build Firmware

Install the Pico SDK and set `PICO_SDK_PATH`, then build:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
```

Flash the generated UF2:

```text
build/probebridge.uf2
```

Open the USB serial port from a terminal. The CDC baud rate selected by the host is nominal; USB CDC does not depend on that value for transport speed.

## Run Host Parser Tests

The parser tests do not need the Pico SDK.

```powershell
cmake -S . -B build-host -G Ninja -DBRIDGE_HOST_TESTS=ON
cmake --build build-host
.\build-host\parser_tests.exe
```

The test executable links `src/command.c` against fake bus functions in `tests/parser_tests.c`. This keeps command grammar and reply formatting testable without hardware.

## Build Documentation

Markdown docs live under `docs/`. The formal manual is at:

```text
docs/latex/manual.tex
```

Build the LaTeX manual with:

```powershell
New-Item -ItemType Directory -Force docs\latex\build
pdflatex -output-directory docs\latex\build docs\latex\manual.tex
```

## Documentation Map

- [Architecture](docs/architecture.md): firmware structure and extension points.
- [Command Reference](docs/command-reference.md): complete command grammar and reply format.
- [Hardware Bring-Up](docs/hardware-bringup.md): wiring, first-run checklist, and troubleshooting.
- [Testing Strategy](docs/testing.md): parser tests, firmware smoke tests, and bench validation.
- [LaTeX Manual](docs/latex/manual.tex): polished technical manual for PDF export.

## Design Notes

- `src/command.c` owns user-facing syntax and validation.
- `src/bus_pico.c` owns RP2040-specific peripheral setup and transactions.
- `include/bridge/bus.h` is the portability boundary for future MCU backends.
- Transfer payloads are capped at 64 bytes to keep RAM use and accidental commands bounded.
- SPI currently uses a fixed chip-select pin and returns one byte for every byte clocked out.

## Roadmap

- Configurable SPI mode and chip-select routing.
- GPIO commands for target reset and fixture control.
- ADC-based target voltage measurement.
- Optional binary framing for faster automated test loops.
- Adapter PCB with pull-ups, level shifting, ESD protection, and labeled headers.
- STM32 backend using the same parser and `bus.h` contract.
