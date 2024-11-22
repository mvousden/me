#ifndef ME_CURSOR_H
#define ME_CURSOR_H

#include "line.h"

/* Controlling/storing the movement of the cursor, very OOP-ish. Origin
 * co-ordinates at (0, 0). Any negative co-ordinate, or any co-ordinate
 * exceeding a bound defined below, is considered out-of-bounds (oob). */
struct Cursor
{
    /* Location of the cursor on the terminal */
    short curLine;
    short curCol;

    /* Cursor bounds */
    short maxLine;
    short maxCol;
};
/* Elementary cursor movement operations */
int cursor_up(struct Cursor* const);
int cursor_dn(struct Cursor* const);
int cursor_lt(struct Cursor* const);
int cursor_rt(struct Cursor* const);

/* Line cursor movement operations */
int cursor_eol(struct Cursor* const, const struct Line* const);
void cursor_sol(struct Cursor* const);
int is_cursor_sol(struct Cursor* const);
int is_cursor_eol(struct Cursor* const, const struct Line* const);

int cursor_oob_check(struct Cursor* const);
void init_cursor(struct Cursor* const);
int warp_cursor(struct Cursor* const, const short, const short);
int update_cursor_max_bounds(struct Cursor* const, const short, const short);
#endif
