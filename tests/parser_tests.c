#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bridge/bus.h"
#include "bridge/command.h"

typedef struct {
    char text[4096];
    size_t len;
} capture_t;

static bridge_config_t g_config = {
    .i2c_baud = 100000,
    .spi_baud = 1000000,
    .uart_baud = 115200,
    .uart_bits = 8,
    .uart_stop_bits = 1,
};

static void capture_write(const char *text, void *ctx) {
    capture_t *capture = (capture_t *)ctx;
    size_t room = sizeof(capture->text) - capture->len - 1;
    size_t n = strlen(text);
    if (n > room) {
        n = room;
    }
    memcpy(capture->text + capture->len, text, n);
    capture->len += n;
    capture->text[capture->len] = '\0';
}

static void run_command(const char *input, capture_t *capture) {
    char line[160];
    snprintf(line, sizeof(line), "%s", input);
    capture->len = 0;
    capture->text[0] = '\0';
    bridge_console_t console = {
        .write = capture_write,
        .ctx = capture,
    };
    bridge_command_execute(line, &console);
}

static void expect_contains(const char *name, const char *haystack, const char *needle) {
    if (strstr(haystack, needle) == NULL) {
        fprintf(stderr, "FAIL %s: expected '%s' in '%s'\n", name, needle, haystack);
        exit(1);
    }
}

void bridge_bus_init(void) {
}

const bridge_config_t *bridge_bus_config(void) {
    return &g_config;
}

bridge_status_t bridge_i2c_set_baud(uint32_t baud) {
    g_config.i2c_baud = baud;
    return BRIDGE_OK;
}

bridge_status_t bridge_spi_set_baud(uint32_t baud) {
    g_config.spi_baud = baud;
    return BRIDGE_OK;
}

bridge_status_t bridge_uart_set_baud(uint32_t baud) {
    g_config.uart_baud = baud;
    return BRIDGE_OK;
}

bridge_status_t bridge_i2c_scan(bridge_i2c_scan_t *scan) {
    scan->count = 2;
    scan->devices[0].address = 0x1a;
    scan->devices[0].present = 1;
    scan->devices[1].address = 0x68;
    scan->devices[1].present = 1;
    return BRIDGE_OK;
}

bridge_status_t bridge_i2c_read(uint8_t address, bridge_bytes_t *rx, size_t len) {
    (void)address;
    rx->len = len;
    for (size_t i = 0; i < len; i++) {
        rx->bytes[i] = (uint8_t)(0xa0 + i);
    }
    return BRIDGE_OK;
}

bridge_status_t bridge_i2c_write(uint8_t address, const bridge_bytes_t *tx) {
    return address == 0x1a && tx->len > 0 ? BRIDGE_OK : BRIDGE_BAD_ARG;
}

bridge_status_t bridge_i2c_write_read(uint8_t address, const bridge_bytes_t *tx, bridge_bytes_t *rx, size_t rx_len) {
    (void)address;
    (void)tx;
    rx->len = rx_len;
    for (size_t i = 0; i < rx_len; i++) {
        rx->bytes[i] = (uint8_t)(0xb0 + i);
    }
    return BRIDGE_OK;
}

bridge_status_t bridge_spi_transfer(const bridge_bytes_t *tx, bridge_bytes_t *rx) {
    rx->len = tx->len;
    for (size_t i = 0; i < tx->len; i++) {
        rx->bytes[i] = (uint8_t)(tx->bytes[i] ^ 0xffu);
    }
    return BRIDGE_OK;
}

bridge_status_t bridge_uart_write(const bridge_bytes_t *tx) {
    return tx->len > 0 ? BRIDGE_OK : BRIDGE_BAD_ARG;
}

bridge_status_t bridge_uart_read(bridge_bytes_t *rx, size_t len, uint32_t timeout_ms) {
    (void)timeout_ms;
    rx->len = len;
    for (size_t i = 0; i < len; i++) {
        rx->bytes[i] = (uint8_t)i;
    }
    return BRIDGE_OK;
}

bridge_status_t bridge_onewire_reset(int *presence) {
    *presence = 1;
    return BRIDGE_OK;
}

bridge_status_t bridge_onewire_write_byte(uint8_t byte) {
    return byte == 0xff ? BRIDGE_BAD_ARG : BRIDGE_OK;
}

bridge_status_t bridge_onewire_read_byte(uint8_t *byte) {
    static uint8_t value = 0x10;
    *byte = value++;
    return BRIDGE_OK;
}

int main(void) {
    capture_t capture = {0};

    run_command("i2c scan", &capture);
    expect_contains("i2c scan", capture.text, "OK 0x1A 0x68");

    run_command("i2c read 0x1a 3", &capture);
    expect_contains("i2c read", capture.text, "OK 0xA0 0xA1 0xA2");

    run_command("i2c write 0x1a 0xff 2 0b1010", &capture);
    expect_contains("i2c write", capture.text, "OK");

    run_command("i2c wr 0x1a 2 0x00", &capture);
    expect_contains("i2c wr", capture.text, "OK 0xB0 0xB1");

    run_command("spi xfer 0x00 0xff", &capture);
    expect_contains("spi xfer", capture.text, "OK 0xFF 0x00");

    run_command("uart read 2 50", &capture);
    expect_contains("uart read", capture.text, "OK 0x00 0x01");

    run_command("ow reset", &capture);
    expect_contains("ow reset", capture.text, "presence=1");

    run_command("ow read 2", &capture);
    expect_contains("ow read", capture.text, "OK 0x10 0x11");

    run_command("i2c read 0x02 1", &capture);
    expect_contains("bad address", capture.text, "ERR usage");

    printf("parser tests passed\n");
    return 0;
}
