#include <string.h>

#include "buffer.h"
#include "conf.h"
#include "error.h"
#include "keyb_defs.h"
#include "state.h"

extern struct MeState state;
extern struct MeConf conf;

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

/* Assumes that the buffer is already empty, and that state.ioFile is opened
 * for reading (at least). Reads the ioFile one chunk at a time, and puts the
 * text into lines, split by '\n'. Call without an empty line setup at your
 * peril. */
void populate_buffer_from_iofile(struct Buffer* const buffer,
                                 FILE* const ioFile)
{
    /* Read the ioFile in one chunk at a time. */
    size_t itRead;  /* Characters from read buffer, also decremented. */
    size_t lineWritten = 0;  /* How many characters written to this line. */
    char* const readBuf = malloc(conf.openChunkSize * sizeof(char));
    char* cFl;  /* Pointer into the read buffer */
    char* cLc = buffer->currentLine->content;  /* Pointer into line content */
    do  /* Reading the entire file on a per-chunk basis. */
    {
        /* Get the chunk */
        itRead = fread(readBuf, sizeof(char), conf.openChunkSize, ioFile);
        if (ferror(ioFile))
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
                buffer->currentLine->len = lineWritten;
                lineWritten = 0;
                if (!(buffer->currentLine->next = malloc(sizeof(struct Line))))
                    err("populate_buffer_from_iofile (OOM)");
                init_line(buffer->currentLine->next,
                          buffer->currentLine, NULL);
                /* The show must go on. */
                buffer->currentLine = buffer->currentLine->next;
                cLc = buffer->currentLine->content;
            }
            cFl++;  /* We just blindly skip characters that we can't print
                     * (except EOF and \n obviously). */
        }
    }
    while (!feof(ioFile));
    /* Need to set an end to the final line */
    buffer->currentLine->len = lineWritten;
    /* Editing experience starts at the top of the file, though note that we
     * have not moved the cursor here. */
    buffer->currentLine = buffer->topLine;
    free(readBuf);
}
