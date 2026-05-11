#include "bridge/bus.h"

#include <stdbool.h>
#include <string.h>

#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"

#define PIN_I2C_SDA 4
#define PIN_I2C_SCL 5
#define PIN_SPI_MISO 16
#define PIN_SPI_CS 17
#define PIN_SPI_SCK 18
#define PIN_SPI_MOSI 19
#define PIN_UART_TX 0
#define PIN_UART_RX 1
#define PIN_ONEWIRE 22

#define UART_READ_POLL_US 1000

static bridge_config_t g_config = {
    .i2c_baud = 100000,
    .spi_baud = 1000000,
    .uart_baud = 115200,
    .uart_bits = 8,
    .uart_stop_bits = 1,
};

static void onewire_drive_low(void) {
    gpio_set_dir(PIN_ONEWIRE, GPIO_OUT);
    gpio_put(PIN_ONEWIRE, 0);
}

static void onewire_release(void) {
    gpio_set_dir(PIN_ONEWIRE, GPIO_IN);
}

void bridge_bus_init(void) {
    i2c_init(i2c0, g_config.i2c_baud);
    gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C_SDA);
    gpio_pull_up(PIN_I2C_SCL);

    spi_init(spi0, g_config.spi_baud);
    gpio_set_function(PIN_SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_MOSI, GPIO_FUNC_SPI);
    gpio_init(PIN_SPI_CS);
    gpio_set_dir(PIN_SPI_CS, GPIO_OUT);
    gpio_put(PIN_SPI_CS, 1);

    uart_init(uart0, g_config.uart_baud);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);
    uart_set_format(uart0, g_config.uart_bits, g_config.uart_stop_bits, UART_PARITY_NONE);

    gpio_init(PIN_ONEWIRE);
    gpio_pull_up(PIN_ONEWIRE);
    onewire_release();
}

const bridge_config_t *bridge_bus_config(void) {
    return &g_config;
}

bridge_status_t bridge_i2c_set_baud(uint32_t baud) {
    if (baud < 10000 || baud > 1000000) {
        return BRIDGE_BAD_ARG;
    }
    g_config.i2c_baud = i2c_set_baudrate(i2c0, baud);
    return BRIDGE_OK;
}

bridge_status_t bridge_spi_set_baud(uint32_t baud) {
    if (baud < 1000 || baud > 32000000) {
        return BRIDGE_BAD_ARG;
    }
    g_config.spi_baud = spi_set_baudrate(spi0, baud);
    return BRIDGE_OK;
}

bridge_status_t bridge_uart_set_baud(uint32_t baud) {
    if (baud < 1200 || baud > 2000000) {
        return BRIDGE_BAD_ARG;
    }
    g_config.uart_baud = uart_set_baudrate(uart0, baud);
    return BRIDGE_OK;
}

bridge_status_t bridge_i2c_scan(bridge_i2c_scan_t *scan) {
    scan->count = 0;
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        uint8_t dummy = 0;
        int rc = i2c_read_timeout_us(i2c0, addr, &dummy, 1, false, 2000);
        if (rc >= 0 && scan->count < 128) {
            scan->devices[scan->count].address = addr;
            scan->devices[scan->count].present = 1;
            scan->count++;
        }
    }
    return BRIDGE_OK;
}

bridge_status_t bridge_i2c_read(uint8_t address, bridge_bytes_t *rx, size_t len) {
    if (len > BRIDGE_MAX_BYTES) {
        return BRIDGE_BAD_ARG;
    }
    rx->len = len;
    int rc = i2c_read_timeout_us(i2c0, address, rx->bytes, len, false, 10000);
    return rc == (int)len ? BRIDGE_OK : BRIDGE_ERR;
}

bridge_status_t bridge_i2c_write(uint8_t address, const bridge_bytes_t *tx) {
    int rc = i2c_write_timeout_us(i2c0, address, tx->bytes, tx->len, false, 10000);
    return rc == (int)tx->len ? BRIDGE_OK : BRIDGE_ERR;
}

bridge_status_t bridge_i2c_write_read(uint8_t address, const bridge_bytes_t *tx, bridge_bytes_t *rx, size_t rx_len) {
    if (rx_len > BRIDGE_MAX_BYTES) {
        return BRIDGE_BAD_ARG;
    }

    int wrc = i2c_write_timeout_us(i2c0, address, tx->bytes, tx->len, true, 10000);
    if (wrc != (int)tx->len) {
        return BRIDGE_ERR;
    }

    rx->len = rx_len;
    int rrc = i2c_read_timeout_us(i2c0, address, rx->bytes, rx_len, false, 10000);
    return rrc == (int)rx_len ? BRIDGE_OK : BRIDGE_ERR;
}

bridge_status_t bridge_spi_transfer(const bridge_bytes_t *tx, bridge_bytes_t *rx) {
    rx->len = tx->len;
    gpio_put(PIN_SPI_CS, 0);
    int rc = spi_write_read_blocking(spi0, tx->bytes, rx->bytes, tx->len);
    gpio_put(PIN_SPI_CS, 1);
    return rc == (int)tx->len ? BRIDGE_OK : BRIDGE_ERR;
}

bridge_status_t bridge_uart_write(const bridge_bytes_t *tx) {
    uart_write_blocking(uart0, tx->bytes, tx->len);
    return BRIDGE_OK;
}

bridge_status_t bridge_uart_read(bridge_bytes_t *rx, size_t len, uint32_t timeout_ms) {
    if (len > BRIDGE_MAX_BYTES) {
        return BRIDGE_BAD_ARG;
    }

    rx->len = 0;
    absolute_time_t deadline = make_timeout_time_ms(timeout_ms);
    while (rx->len < len && !time_reached(deadline)) {
        if (uart_is_readable(uart0)) {
            rx->bytes[rx->len++] = uart_getc(uart0);
        } else {
            sleep_us(UART_READ_POLL_US);
        }
    }
    return rx->len == len ? BRIDGE_OK : BRIDGE_TIMEOUT;
}

bridge_status_t bridge_onewire_reset(int *presence) {
    onewire_drive_low();
    sleep_us(480);
    onewire_release();
    sleep_us(70);
    *presence = gpio_get(PIN_ONEWIRE) == 0;
    sleep_us(410);
    return BRIDGE_OK;
}

bridge_status_t bridge_onewire_write_byte(uint8_t byte) {
    for (int bit = 0; bit < 8; bit++) {
        bool value = (byte >> bit) & 1u;
        onewire_drive_low();
        if (value) {
            sleep_us(6);
            onewire_release();
            sleep_us(64);
        } else {
            sleep_us(60);
            onewire_release();
            sleep_us(10);
        }
    }
    return BRIDGE_OK;
}

bridge_status_t bridge_onewire_read_byte(uint8_t *byte) {
    uint8_t value = 0;
    for (int bit = 0; bit < 8; bit++) {
        onewire_drive_low();
        sleep_us(6);
        onewire_release();
        sleep_us(9);
        if (gpio_get(PIN_ONEWIRE)) {
            value |= (uint8_t)(1u << bit);
        }
        sleep_us(55);
    }
    *byte = value;
    return BRIDGE_OK;
}
