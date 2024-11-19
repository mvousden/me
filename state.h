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
