#include <stdio.h>

#include "pico/stdlib.h"

#include "bridge/bus.h"
#include "bridge/command.h"
#include "bridge/line_editor.h"

#define LINE_BUFFER_LEN 160

static void console_write(const char *text, void *ctx) {
    (void)ctx;
    fputs(text, stdout);
}

int main(void) {
    stdio_init_all();
    bridge_bus_init();

    bridge_console_t console = {
        .write = console_write,
        .ctx = NULL,
    };

    char line_storage[LINE_BUFFER_LEN];
    line_editor_t editor;
    line_editor_init(&editor, line_storage, sizeof(line_storage));

    sleep_ms(1200);
    console.write("\r\nProbeBridge ready\r\n", console.ctx);
    bridge_command_print_help(&console);
    console.write("> ", console.ctx);

    while (true) {
        int ch = getchar_timeout_us(0);
        if (line_editor_push(&editor, ch) == LINE_READY) {
            console.write("\r\n", console.ctx);
            bridge_command_execute(line_storage, &console);
            line_editor_init(&editor, line_storage, sizeof(line_storage));
            console.write("> ", console.ctx);
        } else if (ch >= 0) {
            putchar_raw(ch);
        }
        tight_loop_contents();
    }
}
