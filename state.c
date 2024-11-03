#include <string.h>

#include "conf.h"
#include "keyb_defs.h"
#include "error.h"
#include "state.h"
struct MeState state;
extern struct MeConf conf;

void destroy_state(void)
{
    destroy_line_cascade(state.topLine);
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
    struct Line* lineIt = state.topLine;
    if (!lineIt)  /* Not using error system */
    {
        fprintf(out,
                "There are no lines! Something has gone horribly wrong.\n");
        return;
    }
    while ((lineIt = lineIt->next))
    {
        totalLines++;
        if (lineIt == state.currentLine) currentLine = totalLines;
    }

    /* Header and headline stats */
    fprintf(out,
            "me_dump\n"
            "File Path: %s\n"
            "Current line / line count: %zu/%zu\n"
            "Cursor line, cursor column: %u,%u\n",
            state.filePath,
            currentLine, totalLines,
            state.curLine, state.curCol);

    /* Each line: the entire contents of the buffer including 0. Lines begin
     * with 'L<NUM>:<LEN>:<SIZE>:', where:
     *  - <NUM> is the (1-based) line number,
     *  - <LEN> is the length of the line, from the perspective of the editor,
     *  - <SIZE> is the amount of memory allocated to the content buffer of the
     *    line, and corresponds to the number of bytes following the second
     *    colon.
     * <NUM>, <LEN>, and <SIZE> are printed as ASCII characters, not as raw
     * bytes. */
    lineIt = state.topLine;
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
{
    return state.currentLine->content + state.curCol - conf.colOffset;
}

void init_state(const char* const filePath)
{
    /* File check ahead of time - if filePath is NULL, we ain't reading or
     * writing. */
    if (filePath)
    {
        fclose(fopen(filePath, "a"));  /* Create if not exist. */
        if (!(state.ioFile = fopen(filePath, "r+"))) err("init_state/fopen");
    }

    /* Memory */
    if (!(state.topLine = malloc(sizeof(struct Line))) ||
        (filePath &&
         !(state.filePath = calloc(strlen(filePath) + 1, sizeof(char)))))
        err("init_state (OOM)");

    /* Various setup */
    if(filePath) strcpy(state.filePath, filePath);
    init_line(state.topLine, NULL, NULL);
    state.currentLine = state.topLine;
    state.vt100Buf = malloc(sizeof(char) * conf.vt100BufSize);
    state.curCol = conf.colOffset;
    state.curLine = conf.lineOffset;

    /* Get all text from file and load it into line datastructure. */
    if(filePath) populate_buffer_from_iofile();
}

/* Assumes that the buffer is already empty, and that state.ioFile is opened
 * for reading (at least). Reads the ioFile one chunk at a time, and puts the
 * text into lines, split by '\n'. Call without an empty line setup at your
 * peril. */
void populate_buffer_from_iofile(void)
{
    /* Read the ioFile in one chunk at a time. */
    size_t itRead;  /* Characters from read buffer, also decremented. */
    size_t lineWritten = 0;  /* How many characters written to this line. */
    char* const readBuf = malloc(conf.openChunkSize * sizeof(char));
    char* cFl;  /* Pointer into the read buffer */
    char* cLc = state.currentLine->content;  /* Pointer into line content */
    do  /* Reading the entire file on a per-chunk basis. */
    {
        /* Get the chunk */
        itRead = fread(readBuf, sizeof(char), conf.openChunkSize,
                       state.ioFile);
        if (ferror(state.ioFile))
        {
            free(readBuf);  /* Courtesy */
            err("populate_buffer_from_iofile/fread");
        }
        cFl = readBuf;
        /* Going through each char in the chunk. All chunks are the same size,
         * except ones that run into eof (or errors...). */
        for (; itRead; itRead--)
        {
            /* Writable characters get written, obviously. */
            if (IS_PRINTABLE((unsigned)*cFl))
            {
                *cLc++ = *cFl;
                lineWritten++;
            }
            else if (*cFl == '\n')  /* Here be (baby) dragons */
            {
                /* Here be (baby) dragons - end this line, and connect a new
                 * line on the end of it. */
                state.currentLine->len = lineWritten;
                lineWritten = 0;
                if (!(state.currentLine->next = malloc(sizeof(struct Line))))
                    err("populate_buffer_from_iofile (OOM)");
                init_line(state.currentLine->next, state.currentLine, NULL);
                /* The show must go on. */
                state.currentLine = state.currentLine->next;
                cLc = state.currentLine->content;
            }
            cFl++;  /* We just blindly skip characters that we can't print
                     * (except EOF and \n obviously). */
        }
    }
    while (!feof(state.ioFile));
    /* Need to set an end to the final line */
    state.currentLine->len = lineWritten;
    /* Editing experience starts at the top of the file, though note that we
     * have not moved the cursor here. */
    state.currentLine = state.topLine;
    free(readBuf);
}
