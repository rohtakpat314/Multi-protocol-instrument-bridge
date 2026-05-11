# Examples

This folder is reserved for example command sessions and host-side automation snippets.

## Example I2C Session

```text
status
i2c scan
i2c speed 400000
i2c wr 0x68 1 0x75
```

## Example SPI Flash Session

```text
spi speed 1000000
spi xfer 0x9F 0x00 0x00 0x00
```

## Example 1-Wire ROM Read

```text
ow reset
ow write 0x33
ow read 8
```
