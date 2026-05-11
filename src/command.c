#include "bridge/command.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bridge/bus.h"

#define REPLY_BUFFER_LEN 192

typedef struct {
    char *items[24];
    int count;
} tokens_t;

static void reply(const bridge_console_t *console, const char *fmt, ...) {
    char buffer[REPLY_BUFFER_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    console->write(buffer, console->ctx);
}

static void reply_status(const bridge_console_t *console, bridge_status_t status) {
    switch (status) {
    case BRIDGE_OK:
        reply(console, "OK\r\n");
        break;
    case BRIDGE_TIMEOUT:
        reply(console, "ERR timeout\r\n");
        break;
    case BRIDGE_BAD_ARG:
        reply(console, "ERR bad argument\r\n");
        break;
    default:
        reply(console, "ERR bus failure\r\n");
        break;
    }
}

static int equals(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

static tokens_t tokenize(char *line) {
    tokens_t tokens = {0};
    char *cursor = line;
    while (*cursor != '\0' && tokens.count < (int)(sizeof(tokens.items) / sizeof(tokens.items[0]))) {
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor == '\0' || *cursor == '#') {
            break;
        }
        tokens.items[tokens.count++] = cursor;
        while (*cursor != '\0' && !isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor != '\0') {
            *cursor++ = '\0';
        }
    }
    return tokens;
}

static bool parse_u32(const char *text, uint32_t min, uint32_t max, uint32_t *out) {
    char *end = NULL;
    unsigned long value = 0;
    if (text[0] == '0' && (text[1] == 'b' || text[1] == 'B')) {
        const char *cursor = text + 2;
        if (*cursor == '\0') {
            return false;
        }
        while (*cursor == '0' || *cursor == '1') {
            value = (value << 1) | (unsigned long)(*cursor - '0');
            cursor++;
        }
        end = (char *)cursor;
    } else {
        value = strtoul(text, &end, 0);
    }
    if (end == text || *end != '\0' || value < min || value > max) {
        return false;
    }
    *out = (uint32_t)value;
    return true;
}

static bool parse_u8(const char *text, uint8_t *out) {
    uint32_t value = 0;
    if (!parse_u32(text, 0, 0xff, &value)) {
        return false;
    }
    *out = (uint8_t)value;
    return true;
}

static bool parse_address7(const char *text, uint8_t *out) {
    uint32_t value = 0;
    if (!parse_u32(text, 0x08, 0x77, &value)) {
        return false;
    }
    *out = (uint8_t)value;
    return true;
}

static bool parse_len(const char *text, size_t *out) {
    uint32_t value = 0;
    if (!parse_u32(text, 0, BRIDGE_MAX_BYTES, &value)) {
        return false;
    }
    *out = (size_t)value;
    return true;
}

static bool parse_bytes(char **items, int count, bridge_bytes_t *out) {
    if (count > BRIDGE_MAX_BYTES) {
        return false;
    }
    out->len = (size_t)count;
    for (int i = 0; i < count; i++) {
        if (!parse_u8(items[i], &out->bytes[i])) {
            return false;
        }
    }
    return true;
}

static void print_bytes(const bridge_console_t *console, const bridge_bytes_t *bytes) {
    for (size_t i = 0; i < bytes->len; i++) {
        reply(console, "%s0x%02X", i == 0 ? "" : " ", bytes->bytes[i]);
    }
    reply(console, "\r\n");
}

static void handle_i2c(const tokens_t *tokens, const bridge_console_t *console) {
    if (tokens->count < 2) {
        reply(console, "ERR usage: i2c scan|read|write|wr|speed\r\n");
        return;
    }

    if (equals(tokens->items[1], "scan")) {
        bridge_i2c_scan_t scan;
        bridge_status_t status = bridge_i2c_scan(&scan);
        if (status != BRIDGE_OK) {
            reply_status(console, status);
            return;
        }
        reply(console, "OK");
        for (size_t i = 0; i < scan.count; i++) {
            reply(console, " 0x%02X", scan.devices[i].address);
        }
        reply(console, "\r\n");
        return;
    }

    if (equals(tokens->items[1], "speed")) {
        uint32_t baud = 0;
        if (tokens->count != 3 || !parse_u32(tokens->items[2], 10000, 1000000, &baud)) {
            reply(console, "ERR usage: i2c speed <hz>\r\n");
            return;
        }
        reply_status(console, bridge_i2c_set_baud(baud));
        return;
    }

    uint8_t address = 0;
    if (tokens->count < 4 || !parse_address7(tokens->items[2], &address)) {
        reply(console, "ERR usage: i2c read <addr> <len> | i2c write <addr> <bytes...> | i2c wr <addr> <rx_len> <bytes...>\r\n");
        return;
    }

    if (equals(tokens->items[1], "read")) {
        size_t len = 0;
        if (tokens->count != 4 || !parse_len(tokens->items[3], &len)) {
            reply(console, "ERR usage: i2c read <addr> <len>\r\n");
            return;
        }
        bridge_bytes_t rx;
        bridge_status_t status = bridge_i2c_read(address, &rx, len);
        if (status == BRIDGE_OK) {
            reply(console, "OK ");
            print_bytes(console, &rx);
        } else {
            reply_status(console, status);
        }
        return;
    }

    if (equals(tokens->items[1], "write")) {
        bridge_bytes_t tx;
        if (!parse_bytes(&tokens->items[3], tokens->count - 3, &tx) || tx.len == 0) {
            reply(console, "ERR usage: i2c write <addr> <bytes...>\r\n");
            return;
        }
        reply_status(console, bridge_i2c_write(address, &tx));
        return;
    }

    if (equals(tokens->items[1], "wr")) {
        size_t rx_len = 0;
        bridge_bytes_t tx;
        bridge_bytes_t rx;
        if (!parse_len(tokens->items[3], &rx_len) || !parse_bytes(&tokens->items[4], tokens->count - 4, &tx)) {
            reply(console, "ERR usage: i2c wr <addr> <rx_len> <bytes...>\r\n");
            return;
        }
        bridge_status_t status = bridge_i2c_write_read(address, &tx, &rx, rx_len);
        if (status == BRIDGE_OK) {
            reply(console, "OK ");
            print_bytes(console, &rx);
        } else {
            reply_status(console, status);
        }
        return;
    }

    reply(console, "ERR unknown i2c command\r\n");
}

static void handle_spi(const tokens_t *tokens, const bridge_console_t *console) {
    if (tokens->count < 2) {
        reply(console, "ERR usage: spi xfer|write|speed\r\n");
        return;
    }

    if (equals(tokens->items[1], "speed")) {
        uint32_t baud = 0;
        if (tokens->count != 3 || !parse_u32(tokens->items[2], 1000, 32000000, &baud)) {
            reply(console, "ERR usage: spi speed <hz>\r\n");
            return;
        }
        reply_status(console, bridge_spi_set_baud(baud));
        return;
    }

    if (equals(tokens->items[1], "xfer") || equals(tokens->items[1], "write")) {
        bridge_bytes_t tx;
        bridge_bytes_t rx;
        if (!parse_bytes(&tokens->items[2], tokens->count - 2, &tx) || tx.len == 0) {
            reply(console, "ERR usage: spi xfer <bytes...>\r\n");
            return;
        }
        bridge_status_t status = bridge_spi_transfer(&tx, &rx);
        if (status == BRIDGE_OK) {
            reply(console, "OK ");
            print_bytes(console, &rx);
        } else {
            reply_status(console, status);
        }
        return;
    }

    reply(console, "ERR unknown spi command\r\n");
}

static void handle_uart(const tokens_t *tokens, const bridge_console_t *console) {
    if (tokens->count < 2) {
        reply(console, "ERR usage: uart read|write|speed\r\n");
        return;
    }

    if (equals(tokens->items[1], "speed")) {
        uint32_t baud = 0;
        if (tokens->count != 3 || !parse_u32(tokens->items[2], 1200, 2000000, &baud)) {
            reply(console, "ERR usage: uart speed <baud>\r\n");
            return;
        }
        reply_status(console, bridge_uart_set_baud(baud));
        return;
    }

    if (equals(tokens->items[1], "write")) {
        bridge_bytes_t tx;
        if (!parse_bytes(&tokens->items[2], tokens->count - 2, &tx) || tx.len == 0) {
            reply(console, "ERR usage: uart write <bytes...>\r\n");
            return;
        }
        reply_status(console, bridge_uart_write(&tx));
        return;
    }

    if (equals(tokens->items[1], "read")) {
        size_t len = 0;
        uint32_t timeout_ms = 100;
        bridge_bytes_t rx;
        if (tokens->count < 3 || tokens->count > 4 || !parse_len(tokens->items[2], &len)) {
            reply(console, "ERR usage: uart read <len> [timeout_ms]\r\n");
            return;
        }
        if (tokens->count == 4 && !parse_u32(tokens->items[3], 1, 60000, &timeout_ms)) {
            reply(console, "ERR usage: uart read <len> [timeout_ms]\r\n");
            return;
        }
        bridge_status_t status = bridge_uart_read(&rx, len, timeout_ms);
        if (status == BRIDGE_OK || status == BRIDGE_TIMEOUT) {
            reply(console, status == BRIDGE_OK ? "OK " : "TIMEOUT ");
            print_bytes(console, &rx);
        } else {
            reply_status(console, status);
        }
        return;
    }

    reply(console, "ERR unknown uart command\r\n");
}

static void handle_onewire(const tokens_t *tokens, const bridge_console_t *console) {
    if (tokens->count < 2) {
        reply(console, "ERR usage: ow reset|read|write\r\n");
        return;
    }

    if (equals(tokens->items[1], "reset")) {
        int presence = 0;
        bridge_status_t status = bridge_onewire_reset(&presence);
        if (status == BRIDGE_OK) {
            reply(console, "OK presence=%d\r\n", presence);
        } else {
            reply_status(console, status);
        }
        return;
    }

    if (equals(tokens->items[1], "write")) {
        bridge_bytes_t tx;
        if (!parse_bytes(&tokens->items[2], tokens->count - 2, &tx) || tx.len == 0) {
            reply(console, "ERR usage: ow write <bytes...>\r\n");
            return;
        }
        for (size_t i = 0; i < tx.len; i++) {
            bridge_status_t status = bridge_onewire_write_byte(tx.bytes[i]);
            if (status != BRIDGE_OK) {
                reply_status(console, status);
                return;
            }
        }
        reply(console, "OK\r\n");
        return;
    }

    if (equals(tokens->items[1], "read")) {
        size_t len = 0;
        bridge_bytes_t rx = {0};
        if (tokens->count != 3 || !parse_len(tokens->items[2], &len)) {
            reply(console, "ERR usage: ow read <len>\r\n");
            return;
        }
        for (size_t i = 0; i < len; i++) {
            bridge_status_t status = bridge_onewire_read_byte(&rx.bytes[i]);
            if (status != BRIDGE_OK) {
                reply_status(console, status);
                return;
            }
            rx.len++;
        }
        reply(console, "OK ");
        print_bytes(console, &rx);
        return;
    }

    reply(console, "ERR unknown ow command\r\n");
}

static void handle_status(const bridge_console_t *console) {
    const bridge_config_t *config = bridge_bus_config();
    reply(console, "OK i2c=%lu spi=%lu uart=%lu uart_format=%uN%u\r\n",
          (unsigned long)config->i2c_baud,
          (unsigned long)config->spi_baud,
          (unsigned long)config->uart_baud,
          config->uart_bits,
          config->uart_stop_bits);
}

void bridge_command_print_help(const bridge_console_t *console) {
    reply(console,
          "commands:\r\n"
          "  help\r\n"
          "  status\r\n"
          "  i2c scan | i2c speed <hz> | i2c read <addr> <len>\r\n"
          "  i2c write <addr> <bytes...> | i2c wr <addr> <rx_len> <bytes...>\r\n"
          "  spi speed <hz> | spi xfer <bytes...>\r\n"
          "  uart speed <baud> | uart write <bytes...> | uart read <len> [timeout_ms]\r\n"
          "  ow reset | ow write <bytes...> | ow read <len>\r\n");
}

void bridge_command_execute(char *line, const bridge_console_t *console) {
    tokens_t tokens = tokenize(line);
    if (tokens.count == 0) {
        return;
    }

    if (equals(tokens.items[0], "help") || equals(tokens.items[0], "?")) {
        bridge_command_print_help(console);
    } else if (equals(tokens.items[0], "status")) {
        handle_status(console);
    } else if (equals(tokens.items[0], "i2c")) {
        handle_i2c(&tokens, console);
    } else if (equals(tokens.items[0], "spi")) {
        handle_spi(&tokens, console);
    } else if (equals(tokens.items[0], "uart")) {
        handle_uart(&tokens, console);
    } else if (equals(tokens.items[0], "ow") || equals(tokens.items[0], "onewire")) {
        handle_onewire(&tokens, console);
    } else {
        reply(console, "ERR unknown command\r\n");
    }
}
