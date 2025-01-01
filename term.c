#define _DEFAULT_SOURCE  /* for cfmakeraw */

#include <string.h>
#include <sys/ioctl.h>
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
    unsigned short line;
    char* slidingBuf = buf;  /* Sliding buffer */
    if (conf.fciCol < state.cursor.maxCol)
    {
        /* Precomputing: stored persistently across drawings */
        if (!state.fciStr)
        {
            size_t bytes = snprintf(0, 0, "%s%c%s", conf.fciStrPre,
                                    conf.fciChar, conf.fciStrPost) + 1;
            if (!(state.fciStr = malloc(bytes))) err("draw_fci (OOM)");
            snprintf(state.fciStr, bytes, "%s%c%s", conf.fciStrPre,
                     conf.fciChar, conf.fciStrPost);
            state.fciStrLen = strlen(state.fciStr);
        }

        /* Staging movements and drawings to buffer */
        slidingBuf = vt100_cursor_pos_to_buf(slidingBuf, 0, conf.fciCol);
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
    struct Line* curLine;
    size_t curLineNum;

    /* Clear screen and draw fci */
    char* slidingBuf = state.vt100Buf;
    update_window_size();
    slidingBuf = slide_copy(VT100_CURSOR_HIDE, slidingBuf);
    slidingBuf = slide_copy(VT100_ERASE_IN_DISPLAY_ALL, slidingBuf);
    slidingBuf = slide_copy(VT100_CURSOR_0_0, slidingBuf);
    slidingBuf = stage_draw_fci(slidingBuf);
    slidingBuf = slide_copy(VT100_CURSOR_0_0, slidingBuf);
    *slidingBuf = 0;
    vt100_exec(state.vt100Buf);

    /* then actual text (so it overlaps the fci) */
    curLine = state.buffer.topLine;
    curLineNum = 0;
    /* Jump to head line inefficiently <!> */
    while (curLineNum != state.headLineNum)
    {
        curLine = curLine -> next;
        curLineNum++;
    }
    /* write */
    do
    {
        if (curLineNum != state.headLineNum) printf("\n");
        printf("%s", curLine->content);
        curLine = curLine->next;
        curLineNum++;
    }
    while (curLine &&
           curLineNum - state.headLineNum < (size_t)state.cursor.maxLine);
    fflush(stdout);

    /* reset the cursor */
    slidingBuf = state.vt100Buf;
    slidingBuf = vt100_cursor_pos_to_buf(slidingBuf,
        state.cursor.curLine + conf.lineOffset,
        state.cursor.curCol + conf.colOffset);
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
    update_cursor_max_bounds(&state.cursor, ioctlOut.ws_row, ioctlOut.ws_col);
}

#undef _DEFAULT_SOURCE
