#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "error.h"
#include "line.h"
extern struct MeConf conf;

void append_char(struct Line* const restrict line, const char in)
{
    char* c;
    check_expand_line_buffer(line, 1);
    c = line->content + line->len;
    c[0] = in;
    c[1] = 0;
    line->len++;
}

/* If the string is not null-terminated, you get what you deserve. */
void append_string(struct Line* const restrict line,
                   const char* const restrict in)
{
    size_t inLen = strlen(in);
    check_expand_line_buffer(line, inLen + 1);
    strcpy(line->content + line->len, in);
    line->len += inLen;
}

/* If inserting `insertLen` chars into the `line->content` buffer would cause
 * it to overflow, expand that buffer.  */
void check_expand_line_buffer(struct Line* const restrict line,
                              const size_t insertLen)
{
    size_t initBufSiz;
    char* c;
    /* Shortcuts */
    if (line->len + insertLen < line->bufSiz || !insertLen) return;
    initBufSiz = line->bufSiz;
    while (line->len + insertLen >= line->bufSiz)
        line->bufSiz += conf.initLineSize;
    line->content = realloc(line->content, sizeof(char) * line->bufSiz);
    if (!line->content) err("check_expand_line_buffer (OOM)");
    /* Initialise new content with zeroes, stops valgrind screaming when we do
     * a state-dump, and it takes very little time to do... */
    for (c = line->content + initBufSiz; c < line->content + line->bufSiz;
         *c++ = 0);
}

void delete_char_from_line(struct Line* const restrict line, const size_t off)
{
    char* c;
    if (off >= line->len) return;  /* Sanity, including wraparound */
    c = line->content + off;  /* Pointer to char to delete */
    /* Chars go backwards. Better than memmove here, also moves 0s. */
    while (c++ != line->content + line->len) *(c - 1) = *c;
    line->len--;
}

void destroy_line(struct Line* line)
{
    free(line->content);
    free(line);
}

void destroy_line_cascade(struct Line* line)
{
    struct Line* nextLine;
    do
    {
        nextLine = line->next;
        destroy_line(line);
        line = nextLine;
    }
    while (nextLine);
}

void init_line(struct Line* const restrict line,
               struct Line* const restrict prev,
               struct Line* const restrict next)
{
    line->prev = prev;
    line->next = next;
    line->bufSiz = conf.initLineSize;
    line->len = 0;
    if (!(line->content = calloc(line->bufSiz, sizeof(char))))
        err("init_line (OOM)");
    *(line->content) = 0;
}

void insert_char_into_line(struct Line* const restrict line, const char in,
                           const size_t off)  /* char prepended if off == 0 */
{
    char* c;
    /* Append instead if offset is too big. */
    if (off > line->len)
    {
        append_char(line, in);
        return;
    }
    check_expand_line_buffer(line, 1);
    /* Now we can guarantee that there's a char after the final 0. */
    c = line->content + line->len + 1;  /* Here's a pointer to it. */
    /* Chars go forward. Better than memmove here, also moves 0s. */
    do *c = *(c - 1); while (--c != line->content + off);
    *c = in;
    line->len++;
}

void merge_line_with_next(struct Line* const restrict line)
{
    struct Line* const markedForDeath = line->next;
    if (!markedForDeath) return;  /* Do nothing if no next line exists */
    /* Append old line content to new line. */
    append_string(line, line->next->content);
    /* Cleanup cleared line. */
    line->next = markedForDeath->next;
    destroy_line(markedForDeath);
}

/* Splits the contents of this line at an offset, moving the rhs contents to a
 * new line placed after this one. Adds whitespace to that line if
 * necessary. */
void split_line(struct Line* const restrict line, const size_t off,
                const char ws, unsigned wsCount)
{
    /* Something to hold a string of repeated whitespace characters, so that it
     * can be appended to the new line efficiently. */
    char* const wsStr = malloc((wsCount + 1) * sizeof(char));
    /* Insert new line */
    struct Line* oldNext = line->next;
    struct Line* newLine;
    if (!(newLine = malloc(sizeof(struct Line)))) err("split_line (OOM)");
    if (oldNext) oldNext->prev = newLine;
    line->next = newLine;
    init_line(newLine, line, oldNext);
    /* Add whitespace to new line, and terminate it. */
    for (wsStr[wsCount] = 0; wsCount; wsStr[--wsCount] = ws);
    append_string(newLine, wsStr);
    /* Copy text from cursor to new line */
    append_string(newLine, line->content + off);
    /* Truncate old line */
    line->content[off] = 0;
    line->len = off;
    free(wsStr);
}
