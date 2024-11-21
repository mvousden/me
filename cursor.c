#include "conf.h"
#include "cursor.h"
#include "state.h"

extern struct MeConf conf;
extern struct MeState state;

/* These int functions return 0 if the cursor remains in-bounds, and 1
 * otherwise, regardless of where the cursor starts. If this value is not
 * checked, cursor operations will likely continue while the cursor is
 * offscreen.
 *
 * Trust the linker to do some of the heavy lifting here. */
int cursor_up(struct Cursor* const restrict c)
{return c->curLine--, cursor_oob_check(c);}
int cursor_dn(struct Cursor* const restrict c)
{return c->curLine++, cursor_oob_check(c);}
int cursor_lt(struct Cursor* const restrict c)
{return c->curCol--, cursor_oob_check(c);}
int cursor_rt(struct Cursor* const restrict c)
{return c->curCol++, cursor_oob_check(c);}
int cursor_eol(struct Cursor* const restrict c,
               const struct Line* const restrict l)
{return c->curCol = l->len + conf.colOffset, cursor_oob_check(c);}
int cursor_sol(struct Cursor* const restrict c)
{return c->curCol = conf.colOffset, cursor_oob_check(c);}


void init_cursor(struct Cursor* const restrict c,
                 const unsigned short curLine,
                 const unsigned short curCol,
                 const unsigned short minLine,
                 const unsigned short minCol)
{
    c->curLine = curLine;
    c->curCol = curCol;
    /* Maximum values set when screen is first drawn. In headless mode
     * (i.e. for testing), these should be set via
     * update_cursor_bounds. Minimum values are fixed in conf. */
    c->minLine = minLine;
    c->minCol = minCol;
}

int cursor_oob_check(struct Cursor* const restrict c)
{
    return (c->curLine > c->maxLine || c->curLine < c->minLine ||
            c->curCol > c->maxCol || c->curCol < c->minCol);
}

int update_cursor_max_bounds(struct Cursor* const restrict c,
                             const unsigned short maxLine,
                             const unsigned short maxCol)
{
    c->maxLine = maxLine;
    c->maxCol = maxCol;
    return cursor_oob_check(c);
}
