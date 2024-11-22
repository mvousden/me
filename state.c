#include <string.h>

#include "conf.h"
#include "keyb_defs.h"
#include "error.h"
#include "state.h"
struct MeState state;
extern struct MeConf conf;

void destroy_state(void)
{
    destroy_buffer_content(&state.buffer);
    if (state.fciStr) free(state.fciStr);
    if (state.filePath) free(state.filePath);
    if (state.ioFile) fclose(state.ioFile);
    free(state.vt100Buf);
}

/* Not a comprehensive dump - used as a debugging tool. */
void dump_state(FILE* const out)
{
    size_t currentLine = 1;
    size_t totalLines = 1;  /* There must always be one! */

    /* Get line count data. */
    struct Line* lineIt = state.buffer.topLine;
    if (!lineIt)  /* Not using error system */
    {
        fprintf(out,
                "There are no lines! Something has gone horribly wrong.\n");
        return;
    }
    while ((lineIt = lineIt->next))
    {
        totalLines++;
        if (lineIt == state.buffer.currentLine) currentLine = totalLines;
    }

    /* Header and headline stats */
    fprintf(out,
            "me_dump\n"
            "File Path: %s\n"
            "Current line / line count: %zu/%zu\n"
            "Cursor line, cursor column: %u,%u\n"
            "Terminal size (cols x lines): %u x %u\n",
            state.filePath,
            currentLine, totalLines,
            state.cursor.curLine, state.cursor.curCol,
            state.cursor.maxCol, state.cursor.maxLine);

    /* Each line: the entire contents of the buffer including 0. Lines begin
     * with 'L<NUM>:<LEN>:<SIZE>:', where:
     *  - <NUM> is the (1-based) line number,
     *  - <LEN> is the length of the line, from the perspective of the editor,
     *  - <SIZE> is the amount of memory allocated to the content buffer of the
     *    line, and corresponds to the number of bytes following the second
     *    colon.
     * <NUM>, <LEN>, and <SIZE> are printed as ASCII characters, not as raw
     * bytes. */
    lineIt = state.buffer.topLine;
    currentLine = 1;  /* reuse */
    do
    {
        fprintf(out, "L%zu:%zu:%zu:", currentLine, lineIt->len,
                lineIt->bufSiz);
        fwrite(lineIt->content, sizeof(char), lineIt->bufSiz, out);
        currentLine++;
    }
    while ((lineIt = lineIt->next));
}

char* get_cp_at_cursor(void)
{return state.buffer.currentLine->content + state.cursor.curCol;}

void init_state(const char* const filePath)
{
    /* File check ahead of time - if filePath is NULL, we ain't reading or
     * writing. */
    if (filePath)
    {
        fclose(fopen(filePath, "a"));  /* Create if not exist. */
        if (!(state.ioFile = fopen(filePath, "r+"))) err("init_state/fopen");
    }

    /* Storing the path to the file. */
    if (filePath &&
        !(state.filePath = calloc(strlen(filePath) + 1, sizeof(char))))
        err("init_state (OOM)");
    if (filePath) strcpy(state.filePath, filePath);

    /* Various setup */
    init_buffer(&state.buffer);
    init_cursor(&state.cursor);
    state.vt100Buf = malloc(sizeof(char) * conf.vt100BufSize);

    /* Get all text from file and load it into line datastructure. */
    if (filePath) populate_buffer_from_iofile(&state.buffer, state.ioFile);
}
