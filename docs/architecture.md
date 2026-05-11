# Architecture

The bridge is organized around a narrow command layer and a replaceable bus backend. The current backend targets Raspberry Pi Pico / RP2040 through the Pico SDK.

## Data Flow

```text
PC terminal or script
        |
        | USB CDC text lines
        v
stdio USB
        |
        v
line_editor.c
        |
        v
command.c
        |
        v
bus.h interface
        |
        v
bus_pico.c
        |
        v
I2C / SPI / UART / 1-Wire pins
```

## Modules

| Module | Responsibility |
| --- | --- |
| `src/main.c` | Boots USB stdio, initializes buses, and runs the command loop. |
| `src/line_editor.c` | Collects characters into newline-terminated command strings. |
| `src/command.c` | Tokenizes commands, validates arguments, dispatches protocol operations, and formats replies. |
| `include/bridge/bus.h` | Defines the bus abstraction used by the parser and tests. |
| `src/bus_pico.c` | Implements the bus abstraction with RP2040 I2C, SPI, UART, GPIO, and timing APIs. |
| `tests/parser_tests.c` | Builds the parser on a host machine with fake bus functions. |

## Design Goals

- Keep the command parser portable and host-testable.
- Keep hardware-specific code isolated behind `bus.h`.
- Prefer readable text commands over binary framing for bench usability.
- Bound transfer sizes to avoid accidental large allocations or long blocking commands.
- Return predictable machine-readable status prefixes: `OK`, `ERR`, and `TIMEOUT`.

## Current Limits

- Maximum transfer size is 64 bytes.
- SPI uses one fixed chip-select pin per transfer.
- UART framing is fixed at 8N1 in the current firmware.
- 1-Wire is implemented with blocking bit-banged timing.
- No voltage translation or target power control is implemented in firmware.

## Extension Points

Good next additions:

- Configurable SPI mode and chip-select pin.
- GPIO commands for reset lines and fixture control.
- Binary command mode for high-throughput automation.
- Target voltage sensing through ADC.
- Scriptable macros stored in flash.
- STM32 backend implementing the same `bus.h` interface.
