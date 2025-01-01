#ifndef ME_STATE_H
#define ME_STATE_H
#include <stddef.h>
#include <stdlib.h>
#include <termios.h>

#include "buffer.h"
#include "cursor.h"
struct MeState
{
    /* Aggregators */
    struct Buffer buffer;
    struct Cursor cursor;
    struct termios termiosOld;  /* Not one of mine */

    /* The line number that corresponds to the top line of the terminal
     * window. Note that the number is chosen here over the pointer to the line
     * to make synchronisation easier - we don't want to check if this line is
     * deleted every time it is drawn. */
    size_t headLineNum;

    /* File I/O */
    char* filePath;
    FILE* ioFile;

    /* Fill column indicator */
    char* fciStr;
    size_t fciStrLen;

    /* vt100 buffer, used for buffering command sequences */
    char* vt100Buf;
};
void destroy_state(void);
void dump_state(FILE* const);
char* get_cp_at_cursor(void);
void init_state(const char* const);
#endif
