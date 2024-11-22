#include "cursor.h"

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
{return c->curCol = l->len, cursor_oob_check(c);}
/* No return, because it must be in-bounds by definition. */
void cursor_sol(struct Cursor* const restrict c){c->curCol = 0;}

/* Boolean-style checks */
int is_cursor_sol(struct Cursor* const c){return !(c->curCol);}
int is_cursor_eol(struct Cursor* const c, const struct Line* const l)
{return c->curCol >= (short)l->len;}

void init_cursor(struct Cursor* const restrict c)
{c->curLine = 0, c->curCol = 0;}

int warp_cursor(struct Cursor* const restrict c, const short curLine,
                const short curCol)
{return c->curCol = curCol, c->curLine = curLine, cursor_oob_check(c);}

int cursor_oob_check(struct Cursor* const restrict c)
{
    return c->curLine > c->maxLine || c->curLine < 0 ||
           c->curCol > c->maxCol || c->curCol < 0;
}

int update_cursor_max_bounds(struct Cursor* const restrict c,
                             const short maxLine, const short maxCol)
{return c->maxLine = maxLine, c->maxCol = maxCol, cursor_oob_check(c);}
