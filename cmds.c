#include <limits.h>
#include <stdio.h>

#include "conf.h"
#include "cmds.h"
#include "error.h"
#include "helpers.h"
#include "line.h"
#include "state.h"

extern struct MeState state;
extern struct MeConf conf;

int cmd_delete_char(const int cursorOff)
{
    /* Backspace at start */
    if (state.cursor.curCol + cursorOff < conf.colOffset)
    {
        if (!state.buffer.currentLine->prev) return 1;  /* Top line */
        state.cursor.curCol = state.buffer.currentLine->prev->len +
            conf.colOffset;
        state.buffer.currentLine = state.buffer.currentLine->prev;
        merge_line_with_next(state.buffer.currentLine);
        state.cursor.curLine--;
    }
    else  /* Delete or (backspace not at start) */
    {
        state.cursor.curCol += cursorOff;
        /* Delete at end */
        if (state.cursor.curCol > state.buffer.currentLine->len)
        {
            if (!state.buffer.currentLine->next) return 1;  /* Bottom line */
            merge_line_with_next(state.buffer.currentLine);
        }
        else  /* Delete not at end, or backspace not at start */
        {
            delete_char_from_line(state.buffer.currentLine,
                                  state.cursor.curCol - conf.colOffset);
        }
    }
    return 1;
}

int cmd_dump_state(void)
{
    FILE* const out = fopen("me_dump", "w");
    if (!out) err("cmd_dump_state/fopen");
    dump_state(out);
    fclose(out);
    return 1;
}

int cmd_insert_char(const unsigned in)
{
    unsigned short max;
    insert_char_into_line(state.buffer.currentLine, (char)in,
                          state.cursor.curCol - conf.colOffset);
    max = state.buffer.currentLine->len + conf.colOffset;
    state.cursor.curCol = state.cursor.curCol < max ?
        state.cursor.curCol + 1 : max;
    return 1;
}

int cmd_move_chars_left(unsigned howMany, unsigned* const isEof)
{
    unsigned eof = 0;
    while (howMany-- && !eof)
    {
        /* Within line */
        if (state.cursor.curCol > conf.colOffset) state.cursor.curCol--;
        else if (state.buffer.currentLine->prev)  /* To end of previous line */
        {
            state.buffer.currentLine = state.buffer.currentLine->prev;
            state.cursor.curLine--;
            state.cursor.curCol = state.buffer.currentLine->len +
                conf.colOffset;
        }
        else eof = 1; /* Top of file */
    }
    if (isEof) *isEof = eof;
    return 1;
}

int cmd_move_chars_right(unsigned howMany, unsigned* const isEof)
{
    unsigned eof = 0;
    while (howMany-- && !eof)
    {
        /* Within line */
        if (state.cursor.curCol - (size_t)conf.colOffset <
            state.buffer.currentLine->len)
            state.cursor.curCol++;
        else if (state.buffer.currentLine->next)  /* To start of next line */
        {
            state.buffer.currentLine = state.buffer.currentLine->next;
            state.cursor.curLine++;
            state.cursor.curCol = conf.colOffset;
        }
        else eof = 1; /* Bottom of file */
    }
    if (isEof) *isEof = eof;
    return 1;
}

int cmd_move_doc_end(void)
{
    cmd_move_lines_down(UINT_MAX);
    cmd_move_line_end();
    return 1;
}

int cmd_move_doc_home(void)
{
    cmd_move_lines_up(UINT_MAX);
    cmd_move_line_home();
    return 1;
}

int cmd_move_line_end(void)
{
    state.cursor.curCol = state.buffer.currentLine->len + 1;
    return 1;
}

int cmd_move_line_home(void)
{
    state.cursor.curCol = conf.colOffset;
    return 1;
}

int cmd_move_lines_down(unsigned howMany)
{
    while (howMany--)
    {
        if (state.buffer.currentLine->next)
        {
            state.buffer.currentLine = state.buffer.currentLine->next;
            state.cursor.curLine++;
        }
        else break;
    }
    return 1;
}

int cmd_move_lines_up(unsigned howMany)
{
    while (howMany--)
    {
        if (state.buffer.currentLine->prev)
        {
            state.buffer.currentLine = state.buffer.currentLine->prev;
            state.cursor.curLine--;
        }
        else break;
    }
    return 1;
}

/* I realise this reads awfully, but it's the only way I can make this somewhat
 * palatable - it's frought with edge cases. Sorry about that. */
int cmd_move_word(const unsigned isRight)
{
    const char* c;
    unsigned eof = 0;
    /* Backward by one step if we are on the first character of a word,
     * regardless of where that word is in the line:
     *  - Only if moving left
     *  - Only if this character is part of a word
     *  - Only if (we're at the start of the line) OR (the previous character
     *    is not part of a word). */
    c = get_cp_at_cursor();
    if (!isRight && is_alphanum(*c) &&
        (state.cursor.curCol == conf.colOffset || !is_alphanum(*(c-1))))
    {
        move_char(0, &eof);
        if (eof) return 1;  /* Start of document, shortcut */
    }
    /* Forward/backward until we hit the start/end of a word. If we hit the
     * end/start of the buffer, just leave. */
    while (!is_alphanum(*get_cp_at_cursor()))
    {
        move_char(isRight, &eof);
        if (eof) return 1;  /* End/start of document */
    }
    /* Forward/backward until we hit the end/start of a word. If we hit the
     * end/start of the buffer, just leave. */
    while (is_alphanum(*get_cp_at_cursor()))
    {
        move_char(isRight, &eof);
        if (eof) return 1;  /* End/start of document */
    }
    /* Backward movement must end at the start of the word, not the character
     * before it. */
    if (!isRight) move_char(1, NULL);
    return 1;
}

int cmd_move_word_left(void){return cmd_move_word(0);}
int cmd_move_word_right(void){return cmd_move_word(1);}

/* Stops writing a line when a 0 is encountered in that line - no hidden
 * characters are written. */
int cmd_save_file(void)
{
    struct Line* writeLine = state.buffer.topLine;
    char newl = '\n';
    FILE* const out = fopen(state.filePath, "w");
    if (!out) err("cmd_save_file/fopen");
    do
    {
        if (writeLine->len > 0 &&
            !fwrite(writeLine->content, sizeof(char) * writeLine->len, 1, out))
            err("cmd_save_file:content/fwrite");
        if ((writeLine = writeLine->next))
            if (!fwrite(&newl, sizeof(char), 1, out))
                err("cmd_save_file:newl/fwrite");
    }
    while (writeLine);
    fclose(out);
    return 1;
}

int cmd_split_line(const char ws, const unsigned wsCount)
{
    split_line(state.buffer.currentLine,
               state.cursor.curCol - conf.colOffset, ws, wsCount);
    state.cursor.curCol = conf.colOffset + wsCount;
    state.cursor.curLine++;
    state.buffer.currentLine = state.buffer.currentLine->next;
    return 1;
}

int cmd_quit(void){return 0;}

/* Convenience */
int move_char(const unsigned isRight, unsigned* const isEof)
{
    if (isRight) return cmd_move_chars_right(1, isEof);
    else return cmd_move_chars_left(1, isEof);
}
