#ifndef ME_CURSOR_H
#define ME_CURSOR_H
/* Controlling/storing the movement of the cursor, very OOP-ish. */
struct Cursor
{
    /* Location of the cursor on the terminal */
    unsigned short curLine;
    unsigned short curCol;

    /* Cursor bounds */
    unsigned short minLine;
    unsigned short minCol;
    unsigned short maxLine;
    unsigned short maxCol;
};
int cursor_up(struct Cursor* const);
int cursor_dn(struct Cursor* const);
int cursor_lt(struct Cursor* const);
int cursor_rt(struct Cursor* const);

int cursor_oob_check(struct Cursor* const);
void init_cursor(struct Cursor* const, const unsigned short,
                 const unsigned short, const unsigned short,
                 const unsigned short);
int update_cursor_max_bounds(struct Cursor* const, const unsigned short,
                             const unsigned short);
#endif
