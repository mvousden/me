#define _DEFAULT_SOURCE  /* for cfmakeraw */

#include <string.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <unistd.h>

#include "conf.h"
#include "error.h"
#include "helpers.h"
#include "state.h"
#include "term.h"
#include "vt100_escapes.h"

extern struct MeConf conf;
extern struct MeState state;

void clear_screen(void)
{
    vt100_exec(VT100_CURSOR_HIDE);
    vt100_exec(VT100_ERASE_IN_DISPLAY_ALL);
    vt100_exec(VT100_CURSOR_SHOW);
    vt100_exec(VT100_CURSOR_0_0);
}

/* fci = fill column indicator. Stages commands to a buffer to draw the fci,
 * and returns a pointer to the NULL-terminator of the buffer. Expects the
 * cursor to be at 0,0, and does not reset it. Does not draw the fci if the
 * terminal is too narrow. */
char* stage_draw_fci(char* const buf)
{
    int line;
    char* slidingBuf = buf;  /* Sliding buffer */
    if (conf.fciCol < state.maxCol)
    {
        /* Precomputing: stored persistently across drawings */
        if (!state.fciStr)
        {
            int bytes = snprintf(0, 0, "%s%c%s", conf.fciStrPre,
                                 conf.fciChar, conf.fciStrPost) + 1;
            if (bytes < 0) err("draw_fci (bytecount)");
            if (!(state.fciStr = malloc((size_t)bytes))) err("draw_fci (OOM)");
            snprintf(state.fciStr, (size_t)bytes, "%s%c%s", conf.fciStrPre,
                     conf.fciChar, conf.fciStrPost);
            state.fciStrLen = strlen(state.fciStr);
        }

        /* Staging movements and drawings to buffer */
        slidingBuf = vt100_cursor_pos_to_buf(slidingBuf, 0,
                                             (unsigned)conf.fciCol);
        for (line = 0; line <= state.cursor.maxLine; line++)
        {
            slidingBuf = slide_copy(state.fciStr, slidingBuf);
            slidingBuf = slide_copy(VT100_CURSOR_DN, slidingBuf);
            slidingBuf = slide_copy(VT100_CURSOR_LF, slidingBuf);
        }
    }
    return slidingBuf;
}

void redraw_screen(void)
{
    struct Line* curLine = state.buffer.topLine;
    long curLineNum = state.headLineNum < 0 ? state.headLineNum
        : 0; /* Sanity */

    /* Much of how we write to screen depends on where the cursor is. If the
     * cursor is horizontally-further than the screen displays, then redrawing
     * needs to capture this. */
    update_window_size();
    /* If you consider the contents of a file to exist as a set of (potentially
     * very long) pages joined together horizontally, the value of this
     * variable is the horizontal page number... */
    int const pageOffset = MAX(state.cursor.curCol / state.maxCol,
                               0);  /* Defensive */
    /* ...and this is the corresponding column offset, noting that one
     * character is duplicated across pages. */
    int const colOffset = MAX(pageOffset * state.maxCol - 1, 0);

    /* Clear screen and draw fci, if we are on the first horizontal page. This
     * draws the fci first so that the actual text overlays it naturally. */
    char* slidingBuf = state.vt100Buf;
    slidingBuf = slide_copy(VT100_CURSOR_HIDE, slidingBuf);
    slidingBuf = slide_copy(VT100_ERASE_IN_DISPLAY_ALL, slidingBuf);
    slidingBuf = slide_copy(VT100_CURSOR_0_0, slidingBuf);
    if (!pageOffset)
    {
        slidingBuf = stage_draw_fci(slidingBuf);
        slidingBuf = slide_copy(VT100_CURSOR_0_0, slidingBuf);
    }
    *slidingBuf = 0;
    vt100_exec(state.vt100Buf);

    /* Re-centre the view if the cursor has moved out of bounds. */
    if (cursor_oob_check(&(state.cursor))) centre_on_line();

    /* Jump to the head line, albeit inefficiently <!>, where we start
     * writing. */
    while (curLineNum != state.headLineNum)
    {
        curLine = curLine -> next;
        curLineNum++;
    }

    /* Write each 'visible' line in sequence */
    do
    {
        if (curLineNum != state.headLineNum) putchar('\n');
        if (curLine->len >= (size_t)colOffset)
            write(STDOUT_FILENO,
                  /* Start position */
                  curLine->content + colOffset,
                  /* Number of bytes to write, bearing in mind that we don't
                   * want to write more than we have used in that page, or more
                   * than the width of the terminal */
                  MIN(strlen(curLine->content + colOffset),
                      (size_t)state.maxCol));
        curLine = curLine->next;
        curLineNum++;
    }
    while (curLine && curLineNum - state.headLineNum <= state.cursor.maxLine);
    fflush(stdout);

    /* Reset the vt100 cursor */
    slidingBuf = state.vt100Buf;
    slidingBuf = vt100_cursor_pos_to_buf(slidingBuf,
        (unsigned)(state.cursor.curLine + conf.lineOffset),
        (unsigned)(conf.colOffset + state.cursor.curCol - colOffset));
    slidingBuf = slide_copy(VT100_CURSOR_SHOW, slidingBuf);
    *slidingBuf = 0;
    vt100_exec(state.vt100Buf);
}

void restore_original_termios(void)
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &(state.termiosOld)) == -1)
        err("restore_original_termios/tcsetattr");
}

void store_original_termios(void)
{
    if (tcgetattr(STDIN_FILENO, &(state.termiosOld)) == -1)
        err("store_original_termios/tcsetattr");
}

void term_setup(void)
{
    struct termios uncooking = state.termiosOld;
    cfmakeraw(&uncooking);
    uncooking.c_oflag |= OPOST;
    uncooking.c_cc[VMIN] = 0;
    uncooking.c_cc[VTIME] = 1;  /* Deliberate - 'read' times out. */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &uncooking) == -1)
        err("term_setup/tcsetattr");
}

/* Using ioctl, placing value into cursor struct */
void update_window_size(void)
{
    struct winsize ioctlOut;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ioctlOut))
        err("update_window_size/ioctl(TIOCGWINSZ)");
    /* <!> Check rc */
    update_cursor_maxline(&state.cursor, ioctlOut.ws_row - 1);
    state.maxCol = ioctlOut.ws_col;
}

#undef _DEFAULT_SOURCE
