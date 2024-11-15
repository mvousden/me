#ifndef ME_BUFFER_H
#define ME_BUFFER_H
#include <stdlib.h>

#include "line.h"
/* Buffer as in the textual contents of the editor in memory, with some
 * convenient shortcuts. */
struct Buffer
{
    /* The content itself, both non-NULL in user opration. */
    struct Line* topLine;
    struct Line* currentLine;
};
void destroy_buffer_content(struct Buffer* const);
void init_buffer(struct Buffer* const, const char* const);
void populate_buffer_from_iofile(struct Buffer* const, FILE* const);
#endif
