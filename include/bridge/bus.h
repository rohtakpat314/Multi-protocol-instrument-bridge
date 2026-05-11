#ifndef BRIDGE_BUS_H
#define BRIDGE_BUS_H

#include <stddef.h>
#include <stdint.h>

#define BRIDGE_MAX_BYTES 64

typedef enum {
    BRIDGE_OK = 0,
    BRIDGE_ERR = -1,
    BRIDGE_TIMEOUT = -2,
    BRIDGE_BAD_ARG = -3,
} bridge_status_t;

typedef struct {
    uint8_t bytes[BRIDGE_MAX_BYTES];
    size_t len;
} bridge_bytes_t;

typedef struct {
    uint8_t address;
    int present;
} bridge_i2c_device_t;

typedef struct {
    bridge_i2c_device_t devices[128];
    size_t count;
} bridge_i2c_scan_t;

typedef struct {
    uint32_t i2c_baud;
    uint32_t spi_baud;
    uint32_t uart_baud;
    uint8_t uart_bits;
    uint8_t uart_stop_bits;
} bridge_config_t;

void bridge_bus_init(void);
const bridge_config_t *bridge_bus_config(void);
bridge_status_t bridge_i2c_set_baud(uint32_t baud);
bridge_status_t bridge_spi_set_baud(uint32_t baud);
bridge_status_t bridge_uart_set_baud(uint32_t baud);

bridge_status_t bridge_i2c_scan(bridge_i2c_scan_t *scan);
bridge_status_t bridge_i2c_read(uint8_t address, bridge_bytes_t *rx, size_t len);
bridge_status_t bridge_i2c_write(uint8_t address, const bridge_bytes_t *tx);
bridge_status_t bridge_i2c_write_read(uint8_t address, const bridge_bytes_t *tx, bridge_bytes_t *rx, size_t rx_len);

bridge_status_t bridge_spi_transfer(const bridge_bytes_t *tx, bridge_bytes_t *rx);

bridge_status_t bridge_uart_write(const bridge_bytes_t *tx);
bridge_status_t bridge_uart_read(bridge_bytes_t *rx, size_t len, uint32_t timeout_ms);

bridge_status_t bridge_onewire_reset(int *presence);
bridge_status_t bridge_onewire_write_byte(uint8_t byte);
bridge_status_t bridge_onewire_read_byte(uint8_t *byte);

#endif
