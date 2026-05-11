#ifndef BRIDGE_COMMAND_H
#define BRIDGE_COMMAND_H

#include <stddef.h>

typedef void (*bridge_write_fn)(const char *text, void *ctx);

typedef struct {
    bridge_write_fn write;
    void *ctx;
} bridge_console_t;

void bridge_command_print_help(const bridge_console_t *console);
void bridge_command_execute(char *line, const bridge_console_t *console);

#endif
