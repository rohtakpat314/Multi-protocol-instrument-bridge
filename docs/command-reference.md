# Command Reference

Commands are newline-terminated ASCII text sent over the USB CDC virtual serial port. Arguments are separated by whitespace.

## Number Format

The parser accepts:

| Format | Example |
| --- | --- |
| Decimal | `26` |
| Hex | `0x1A` |
| Octal | `032` |
| Binary | `0b11010` |

## Reply Format

Every command returns a short status line.

| Prefix | Meaning |
| --- | --- |
| `OK` | Command completed successfully. |
| `ERR` | Command failed or arguments were invalid. |
| `TIMEOUT` | Command returned partial data before a timeout. |

Byte data is printed as uppercase hex bytes:

```text
OK 0xA0 0xA1 0xA2
```

## General Commands

### `help`

Prints the command summary.

### `status`

Prints configured bus speeds and UART format.

Example:

```text
status
OK i2c=100000 spi=1000000 uart=115200 uart_format=8N1
```

## I2C Commands

### `i2c scan`

Scans 7-bit addresses from `0x08` through `0x77`.

```text
i2c scan
OK 0x1A 0x68
```

### `i2c speed <hz>`

Sets the I2C bus speed. Valid range: 10 kHz to 1 MHz.

```text
i2c speed 400000
OK
```

### `i2c read <addr> <len>`

Reads bytes from a 7-bit address.

```text
i2c read 0x68 6
OK 0x12 0x34 0x56 0x78 0x9A 0xBC
```

### `i2c write <addr> <bytes...>`

Writes one or more bytes to a 7-bit address.

```text
i2c write 0x1A 0xFF 0x00
OK
```

### `i2c wr <addr> <rx_len> <bytes...>`

Performs a repeated-start write-read transaction.

```text
i2c wr 0x68 1 0x75
OK 0x68
```

## SPI Commands

### `spi speed <hz>`

Sets the SPI clock. Valid range: 1 kHz to 32 MHz.

```text
spi speed 1000000
OK
```

### `spi xfer <bytes...>`

Transfers bytes while chip-select is asserted. SPI is full duplex, so the command always returns received bytes.

```text
spi xfer 0x9F 0x00 0x00 0x00
OK 0xEF 0x40 0x18 0x00
```

`spi write` is accepted as an alias for `spi xfer`.

## UART Commands

### `uart speed <baud>`

Sets the UART baud rate. Valid range: 1200 to 2000000 baud.

```text
uart speed 115200
OK
```

### `uart write <bytes...>`

Writes bytes to UART TX.

```text
uart write 0x55 0x0D 0x0A
OK
```

### `uart read <len> [timeout_ms]`

Reads up to `len` bytes. Timeout defaults to 100 ms.

```text
uart read 4 250
OK 0x01 0x02 0x03 0x04
```

If the timeout expires after partial data arrives:

```text
TIMEOUT 0x01 0x02
```

## 1-Wire Commands

### `ow reset`

Issues a reset pulse and reports whether a device presence pulse was detected.

```text
ow reset
OK presence=1
```

### `ow write <bytes...>`

Writes one or more bytes, least-significant bit first per the 1-Wire protocol.

```text
ow write 0x33
OK
```

### `ow read <len>`

Reads bytes from the bus.

```text
ow read 8
OK 0x28 0xFF 0x64 0x1E 0x93 0x16 0x04 0x5C
```
