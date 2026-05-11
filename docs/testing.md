# Testing Strategy

The project is split so that command parsing can be tested without hardware, while protocol behavior is validated on the bench.

## Host Parser Tests

Build with:

```powershell
cmake -S . -B build-host -G Ninja -DBRIDGE_HOST_TESTS=ON
cmake --build build-host
.\build-host\parser_tests.exe
```

These tests link `src/command.c` against fake bus functions in `tests/parser_tests.c`.

Covered behavior:

- I2C scan formatting.
- I2C read/write/write-read parsing.
- SPI transfer parsing and response formatting.
- UART read parsing with timeout argument.
- 1-Wire reset and read parsing.
- Invalid I2C address rejection.
- Hex, decimal, and binary numeric input.

## Firmware Smoke Tests

After flashing:

```text
help
status
i2c scan
spi xfer 0x00
uart read 1 10
ow reset
```

Expected result: commands should return `OK`, `ERR`, or `TIMEOUT` without locking the console.

## Bench Protocol Tests

| Test | Setup | Command |
| --- | --- | --- |
| I2C scan | Known I2C device at `0x68` | `i2c scan` |
| I2C register read | Device with WHOAMI register | `i2c wr 0x68 1 0x75` |
| SPI JEDEC ID | SPI flash chip | `spi xfer 0x9F 0x00 0x00 0x00` |
| UART loopback | GP0 connected to GP1 | `uart write 0x55` then `uart read 1 100` |
| 1-Wire ROM read | DS18B20 on GP22 | `ow reset`, `ow write 0x33`, `ow read 8` |

## Regression Policy

Before changing parser behavior:

1. Add or update a host parser test.
2. Confirm replies remain stable for scripts.
3. Update `docs/command-reference.md`.
4. Update the LaTeX manual if the user-facing protocol changes.

Before changing hardware behavior:

1. Document pinout or timing changes.
2. Run at least one smoke test per affected protocol.
3. Keep old command names as aliases when practical.
