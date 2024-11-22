/* Somewhat redundant with the keyboard tests, but more introspective. */

#include <limits.h>
#include <stdlib.h>

#include "cursor.h"
#include "unity.h"

void setUp(void){}
void tearDown(void){}

/* Expected motion, in-bounds */
void test_elementary_cursor_motion(void)
{
    struct Cursor c;
    init_cursor(&c);
    warp_cursor(&c, 1, 1);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, update_cursor_max_bounds(&c, 80, 80),
        "Update must return 0 if the cursor is still in-bounds.");

    /* Single-movement */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_rt(&c),
        "'Right' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_dn(&c),
        "'Down' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_lt(&c),
        "'Left' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_up(&c),
        "'Up' cursor movement must return 0 when not out of bounds.");
}

/* Do the cursor move methods return what they should? oob = out-of-bounds. */
void test_elementary_cursor_oob(void)
{
    struct Cursor c;
    init_cursor(&c);

    /* Weird upper bounds, but still okay. */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, update_cursor_max_bounds(&c, 0, 0),
        "Update must return 0 if the cursor is still in-bounds.");

    /* Alternating 1 and 0 cases, moving outside the bounding box */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, cursor_dn(&c),
        "'Down' cursor movement must return 1 when out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_up(&c),
        "'Up' cursor movement must return 0 when returning to bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, cursor_rt(&c),
        "'Right' cursor movement must return 1 when out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_lt(&c),
        "'Left' cursor movement must return 0 when returning to bounds.");

    /* The other corner, negative co-ordinates. */
    init_cursor(&c);
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, cursor_up(&c),
        "'Up' cursor movement must return 1 when out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_dn(&c),
        "'Down' cursor movement must return 0 when returning to bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, cursor_lt(&c),
        "'Left' cursor movement must return 1 when out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_rt(&c),
        "'Right' cursor movement must return 0 when returning to bounds.");
}

/* Expected motion and boolean checks for end-of-line and start-of-line
 * operations, in bounds. */
void test_line_cursor_motion_checks(void)
{
    /* Line setup */
    struct Line* l;
    l = malloc(sizeof(struct Line));
    TEST_ASSERT_NOT_NULL(l);      /* oom check */
    init_line(l, NULL, NULL);

    /* Populate line with some chars */
    for (char in = 'a'; in != 'k'; append_char(l, in++));

    /* Cursor setup */
    struct Cursor c;
    init_cursor(&c);
    update_cursor_max_bounds(&c, 80, 80);

    TEST_ASSERT_MESSAGE(is_cursor_sol(&c),
        "Cursor must be initialised at the origin by default.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, c.curCol,  /* Sanity */
        "Cursor must be initialised at the origin by default.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, c.curLine,
        "Cursor must be initialised at the origin by default.");

    /* sol->sol shouldn't move */
    cursor_sol(&c);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, c.curLine,
        "Cursor must not line-hop when 'sol' is commanded.");
    TEST_ASSERT_MESSAGE(is_cursor_sol(&c),
        "Cursor must not move when 'sol' is commanded at the start of the "
        "line.");

    /* sol->eol should move to end of line */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_eol(&c, l),
        "Cursor must be in bounds if line is short when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, c.curLine,
        "Cursor must not line-hop when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(l->len, c.curCol,
        "Cursor must move to end of line when 'eol' is commanded.");
    TEST_ASSERT_MESSAGE(is_cursor_eol(&c, l),
        "Cursor must move to end of line when 'eol' is commanded.");

    /* eol->eol shouldn't move */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_eol(&c, l),
        "Cursor must be in bounds if line is short when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, c.curLine,
        "Cursor must not line-hop when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(l->len, c.curCol,
        "Cursor must not move when 'sol' is commanded at the start of the "
        "line.");
    TEST_ASSERT_MESSAGE(is_cursor_eol(&c, l),
        "Cursor must move to end of line when 'eol' is commanded.");

    /* eol->sol should move to start of line */
    cursor_sol(&c);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, c.curLine,
        "Cursor must not line-hop when 'sol' is commanded.");
    TEST_ASSERT_MESSAGE(is_cursor_sol(&c),
        "Cursor must move to start of line when 'sol' is commanded.");

    /* When hanging, eol should move to end of line */
    c.curCol = 999;
    TEST_ASSERT_MESSAGE(is_cursor_eol(&c, l),
        "Hanging cursor counts as being 'eol'.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, cursor_eol(&c, l),
        "Cursor must be in bounds if line is short when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, c.curLine,
        "Cursor must not line-hop when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(l->len, c.curCol,
        "Cursor must move to end of line when 'eol' is commanded.");
    TEST_ASSERT_MESSAGE(is_cursor_eol(&c, l),
        "Cursor must move to end of line when 'eol' is commanded.");

    /* Clean up our mess. */
    destroy_line(l);
}

/* Do line movement methods return what they should? Only eol is really
 * relevant here. */
void test_line_cursor_oob(void)
{
    /* Line setup */
    struct Line* l;
    l = malloc(sizeof(struct Line));
    TEST_ASSERT_NOT_NULL(l);      /* oom check */
    init_line(l, NULL, NULL);

    /* Populate line with too many chars */
    for (char in = 'a'; in != 'k'; append_char(l, in++));

    /* Cursor setup - note the max bounds are shorter than the contents of the
     * line.*/
    struct Cursor c;
    init_cursor(&c);
    update_cursor_max_bounds(&c, 1, 1);
    TEST_ASSERT_GREATER_THAN(1, l->len);  /* Test setup */

    /* eol should move us out of bounds */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, cursor_eol(&c, l),
        "Cursor is out of bounds when 'eol' is commanded if line is too long "
        "to handle.");

    destroy_line(l);
}

/* Placing the cursor out of bounds by moving the upper bounds. */
void test_update_oob(void)
{
    struct Cursor c;
    init_cursor(&c);
    /* Weird initial value */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, 6, 7),
        "Warping cursor must return 1 if the bounds are not defined, and "
        "the destination is not the origin.");

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, update_cursor_max_bounds(&c, 6, 7),
        "Update must return 0 if the cursor is still in-bounds.");

    /* An oob update */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, update_cursor_max_bounds(&c, 1, 1),
        "Update must return 1 if the cursor is now out of bounds.");
}

/* Warp tests, setting co-ordinates in bounds and oob. */
void test_warp_oob(void)
{
    struct Cursor c;
    init_cursor(&c);
    const short bound = 80;

    /* Column moves */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, update_cursor_max_bounds(&c, bound, bound),
        "Update must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor_col(&c, bound),
        "Warp-col must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor_col(&c, 0),
        "Warp-col must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor_col(&c, -1),
        "Warp-col must return 1 if the cursor is out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor_col(&c, bound + 1),
        "Warp-col must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor_col(&c, bound),
        "Warp-col must return 0 if the cursor is still in-bounds.");

    /* Line moves */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor_line(&c, bound),
        "Warp-line must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor_line(&c, 0),
        "Warp-line must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor_line(&c, -1),
        "Warp-line must return 1 if the cursor is out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor_line(&c, bound + 1),
        "Warp-line must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor_line(&c, bound),
        "Warp-line must return 0 if the cursor is still in-bounds.");

    /* Col moves when line is oob */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor_line(&c, bound + 1),
        "Warp-line must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor_col(&c, 0),
        "Warp-col must return 1 if the cursor is out of line bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor_line(&c, 0),
        "Warp-line must return 0 if the cursor returns in-bounds.");

    /* Line moves when col is oob */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor_col(&c, bound + 1),
        "Warp-col must return 0 if the cursor is still in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor_line(&c, 0),
        "Warp-line must return 1 if the cursor is out of col bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor_col(&c, 0),
        "Warp-col must return 0 if the cursor returns in-bounds.");

    /* Full warps */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor(&c, 0, 0),
        "Warp must return 0 if the cursor remains in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor(&c, bound, 0),
        "Warp must return 0 if the cursor remains in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor(&c, 0, bound),
        "Warp must return 0 if the cursor remains in-bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, warp_cursor(&c, bound, bound),
        "Warp must return 0 if the cursor remains in-bounds.");

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, -1, 0),
        "Warp must return 1 if the cursor moves out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, 0, -1),
        "Warp must return 1 if the cursor moves out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, -1, bound),
        "Warp must return 1 if the cursor moves out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, bound, -1),
        "Warp must return 1 if the cursor moves out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, bound + 1, 0),
        "Warp must return 1 if the cursor moves out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, 0, bound + 1),
        "Warp must return 1 if the cursor moves out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, bound + 1, bound),
        "Warp must return 1 if the cursor moves out of bounds.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, warp_cursor(&c, bound, bound + 1),
        "Warp must return 1 if the cursor moves out of bounds.");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_elementary_cursor_motion);
    RUN_TEST(test_elementary_cursor_oob);
    RUN_TEST(test_line_cursor_motion_checks);
    RUN_TEST(test_line_cursor_oob);
    RUN_TEST(test_update_oob);
    RUN_TEST(test_warp_oob);
    return UNITY_END();
}
