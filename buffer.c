#include <string.h>

#include "buffer.h"
#include "conf.h"
#include "error.h"
#include "keyb_defs.h"
#include "state.h"

extern struct MeState state;
extern struct MeConf conf;

static size_t count_printable_until_newline(char const*, size_t);

void destroy_buffer_content(struct Buffer* const buffer)
{
    destroy_line_cascade(buffer->topLine);
}

void init_buffer(struct Buffer* const buffer)
{
    /* Something to start from */
    if (!(buffer->topLine = malloc(sizeof(struct Line))))
        err("init_buffer (OOM)");

    /* Other setup */
    init_line(buffer->topLine, NULL, NULL);
    buffer->currentLine = buffer->topLine;
}

/* Reads the inputFile one chunk at a time and populate a buffer with its
 * content. Chunks are delimited by newline characters or a fixed length,
 * whichever occurs first.
 *
 * Call with a non-empty current line at your peril (though this should
 * reasonably work with a non-empty buffer!) . */
void populate_buffer_from_file(struct Buffer* const buffer,
                               FILE* const inputFile)
{
    size_t chunkLen;  /* Characters in this chunk, decrements after each one is
                         read, readable or not. */
    size_t blockLen;  /* Characters in this block, decrements after each
                         readable one is read. */
    char* const readBuf = malloc(conf.openChunkSize * sizeof(char));
    char* cFl;  /* Pointer into the read buffer */
    size_t lcIndex = 0;  /* Index into line content. NB: Not using pointers
                          * here because we may need to realloc if the line
                          * gets too long. */

    if (!(readBuf)) err("populate_buffer_from_file (OOM)");  /* Paranoia */

    do  /* This loop iterates per-chunk, and runs for the entire file. */
    {
        /* Get the chunk */
        chunkLen = fread(readBuf, sizeof(char), conf.openChunkSize, inputFile);
        if (ferror(inputFile))
        {
            free(readBuf);  /* Courtesy */
            err("populate_buffer_from_file/fread");
        }
        cFl = readBuf;

        do  /* This loop iterates per-block, and runs for the entire chunk. */
        {
            /* Given a chunk, count the number of printable characters until
             * either the end of the chunk, or the first newline
             * character. We'll call this region a "block". Given this count,
             * ensure the line buffer has sufficient memory allocated to it to
             * contain that many characters. */
            blockLen = count_printable_until_newline(cFl, chunkLen);
            check_expand_line_buffer(buffer->currentLine, blockLen);

            /* Write each writable character in the block. We just blindly skip
             * characters that we can't print, noting that blockLen only
             * includes printable characters. */
            while (blockLen)
            {
                if (IS_PRINTABLE((unsigned)*cFl))
                {
                    buffer->currentLine->content[lcIndex++] = *cFl;
                    buffer->currentLine->len++;
                    blockLen--;  /* We have written a char */
                }
                cFl++, chunkLen--;  /* Next char */
            }

            /* Control has reached the end of this block. If we have reached a
             * newline character, we end this line connect a new empty line on
             * the end of it, and continue reading the chunk. Otherwise, we
             * read the next chunk. */
            if (chunkLen)
            {
                cFl++, chunkLen--;  /* Next char */
                if (!(buffer->currentLine->next = malloc(sizeof(struct Line))))
                    err("populate_buffer_from_file (OOM)");
                init_line(buffer->currentLine->next,
                          buffer->currentLine, NULL);
                buffer->currentLine = buffer->currentLine->next;
                lcIndex = 0;
            }
        }
        while (chunkLen);
    }
    while (!feof(inputFile));
    /* Editing experience starts at the top of the file, though note that we
     * have not moved the cursor here. */
    buffer->currentLine = buffer->topLine;
    free(readBuf);
}

/* Given a character array, return a count of the number of printable
 * characters from the start until either the end of the buffer (denoted by
 * len), or the first newline character. */
static size_t count_printable_until_newline(char const* readBuf,
                                            size_t len)
{
    size_t out = 0;
    while (len)
    {
        if (*readBuf == '\n') break;  /* Lua-style && */
        if (IS_PRINTABLE((unsigned)*readBuf)) out++;
        readBuf++, len--;
    }
    return out;
}
