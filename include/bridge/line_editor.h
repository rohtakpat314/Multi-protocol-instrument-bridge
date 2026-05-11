#ifndef BRIDGE_LINE_EDITOR_H
#define BRIDGE_LINE_EDITOR_H

#include <stddef.h>

typedef enum {
    LINE_IDLE = 0,
    LINE_READY,
} line_status_t;

typedef struct {
    char *buffer;
    size_t capacity;
    size_t len;
} line_editor_t;

void line_editor_init(line_editor_t *editor, char *buffer, size_t capacity);
line_status_t line_editor_push(line_editor_t *editor, int ch);

#endif
