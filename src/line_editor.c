#include "bridge/line_editor.h"

void line_editor_init(line_editor_t *editor, char *buffer, size_t capacity) {
    editor->buffer = buffer;
    editor->capacity = capacity;
    editor->len = 0;
    if (capacity > 0) {
        buffer[0] = '\0';
    }
}

line_status_t line_editor_push(line_editor_t *editor, int ch) {
    if (ch < 0) {
        return LINE_IDLE;
    }

    if (ch == '\r' || ch == '\n') {
        if (editor->capacity > 0) {
            editor->buffer[editor->len] = '\0';
        }
        return LINE_READY;
    }

    if (ch == '\b' || ch == 0x7f) {
        if (editor->len > 0) {
            editor->len--;
            editor->buffer[editor->len] = '\0';
        }
        return LINE_IDLE;
    }

    if (editor->capacity > 0 && editor->len + 1 < editor->capacity) {
        editor->buffer[editor->len++] = (char)ch;
        editor->buffer[editor->len] = '\0';
    }

    return LINE_IDLE;
}
