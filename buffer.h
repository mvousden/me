#ifndef ME_BUFFER_H
#define ME_BUFFER_H
#include <stdlib.h>

#include "line.h"
/* Buffer as in the textual contents of the editor in memory, with some
 * convenient shortcuts. The buffer's contents are stored as a linked list of
 * Lines. */
struct Buffer
{
    /* The content itself, both non-NULL in user opration. */
    struct Line* topLine;

    /* The line presently holding the cursor. */
    struct Line* currentLine;
};
void destroy_buffer_content(struct Buffer* const);
void init_buffer(struct Buffer* const);
void populate_buffer_from_iofile(struct Buffer* const, FILE* const);
#endif
