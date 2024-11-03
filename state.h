#ifndef ME_STATE_H
#define ME_STATE_H
#include <stddef.h>
#include <stdlib.h>
#include <termios.h>

#include "line.h"
struct MeState
{
    struct termios termiosOld;

    /* File I/O */
    char* filePath;
    FILE* ioFile;

    /* Buffer content - both non-NULL in user operation */
    struct Line* topLine;
    struct Line* currentLine;

    /* The cursor */
    unsigned short curLine;
    unsigned short curCol;

    /* Fill column indicator */
    char* fciStr;
    size_t fciStrLen;

    /* Terminal dimensions */
    unsigned short termLines;
    unsigned short termCols;

    /* vt100 buffer, used for buffering command sequences */
    char* vt100Buf;
};
void destroy_state(void);
void dump_state(FILE* const);
char* get_cp_at_cursor(void);
void init_state(const char* const);
void populate_buffer_from_iofile(void);
#endif
