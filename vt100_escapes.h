/* Not to be included lightly. Values pilfered from
 * https://vt100.net/docs/vt100-ug/chapter3.html */
#define VT100_CURSOR_0_0 "\x1b[0;0H"  /* 0;0 being exactly what you expect */
#define VT100_CURSOR_DN "\x1b[1B"  /* 1 being exactly what you expect */
#define VT100_CURSOR_LF "\x1b[1D"  /* 1 being exactly what you expect */
#define VT100_CURSOR_HIDE "\x1b[?25l"
#define VT100_CURSOR_SHOW "\x1b[?25h"
#define VT100_ERASE_IN_DISPLAY_ALL "\x1b[2J"

#include <stdio.h>
#include <stdlib.h>

/* Execute a bunch of vt100 escape sequences, with other characters thrown in
 * if you want. */
void vt100_exec(const char* const escape)
{
    write(STDOUT_FILENO, escape, strlen(escape));
}

/* Writes a cursor position command to a buffer. It better be big enough!
 * Returns the pointer of dest after the copy. */
char* vt100_cursor_pos_to_buf(char* buf,
                              const unsigned short line,
                              const unsigned short col)
{
    const size_t bytes = snprintf(0, 0, "\x1b[%05u;%05uH", line, col) + 1;
    snprintf(buf, bytes, "\x1b[%05u;%05uH", line, col);
    while (*buf++);  /* snprintf null-terminates */
    return buf - 1;
}
