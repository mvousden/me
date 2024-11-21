/* Somewhat redundant with the keyboard tests, but more introspective. */

#include <limits.h>
#include <stdlib.h>

#include "conf.h"
#include "cursor.h"
#include "unity.h"

extern struct MeConf conf;

void setUp(void){}
void tearDown(void){}

/* Expected motion, in-bounds */
void test_elementary_cursor_motion(void)
{
    struct Cursor c;
    init_cursor(&c, 1, 1, 1, 1);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, update_cursor_max_bounds(&c, 80, 80),
        "Update must return 0 if the cursor is still in-bounds.");

    /* Single-movement */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_rt(&c),
        "'Right' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_dn(&c),
        "'Down' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_lt(&c),
        "'Left' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_up(&c),
        "'Up' cursor movement must return 0 when not out of bounds.");
}

/* Do the cursor move methods return what they should? oob = out-of-bounds. */
void test_elementary_cursor_oob(void)
{
    struct Cursor c;
    init_cursor(&c, 6, 7, 5, 6);  /* Weird lower bounds and initial value */

    /* Weird upper bounds, but still okay. */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, update_cursor_max_bounds(&c, 6, 7),
        "Update must return 0 if the cursor is still in-bounds.");

    /* The bounding box looks like this:

       0XXXXXXXXX
       XXXXXXXXXX
       XXXXXXXXXX
       XXXXXXXXXX
       XXXXXXXXXX
       XXXXXX..XX
       XXXXXX.sXX
       XXXXXXXXXX
       XXXXXXXXXX
       XXXXXXXXXX

     */

    /* Alternating 1 and 0 cases, moving outside the bounding box */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, cursor_dn(&c),
        "'Down' cursor movement must return 1 when out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_up(&c),
        "'Up' cursor movement must return 0 when returning to bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, cursor_rt(&c),
        "'Right' cursor movement must return 1 when out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_lt(&c),
        "'Left' cursor movement must return 0 when returning to bounds.");

    /* The other corner */
    init_cursor(&c, 5, 6, 5, 6);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, cursor_up(&c),
        "'Up' cursor movement must return 1 when out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_dn(&c),
        "'Down' cursor movement must return 0 when returning to bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, cursor_lt(&c),
        "'Left' cursor movement must return 1 when out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_rt(&c),
        "'Right' cursor movement must return 0 when returning to bounds.");
}

/* Expected motion for end-of-line and start-of-line operations, in bounds. */
void test_line_cursor_motion(void)
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
    init_cursor(&c, conf.lineOffset, conf.colOffset, conf.lineOffset,
                conf.colOffset);
    update_cursor_max_bounds(&c, 80, 80);

    /* sol->sol shouldn't move */
    cursor_sol(&c);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(conf.lineOffset, c.curLine,
        "Cursor must not line-hop when 'sol' is commanded.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(conf.colOffset, c.curCol,
        "Cursor must not move when 'sol' is commanded at the start of the "
        "line.");

    /* sol->eol should move to end of line */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_eol(&c, l),
        "Cursor must be in bounds if line is short when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(conf.lineOffset, c.curLine,
        "Cursor must not line-hop when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(l->len + conf.colOffset, c.curCol,
        "Cursor must move to end of line when 'eol' is commanded.");

    /* eol->eol shouldn't move */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_eol(&c, l),
        "Cursor must be in bounds if line is short when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(conf.lineOffset, c.curLine,
        "Cursor must not line-hop when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(l->len + conf.colOffset, c.curCol,
        "Cursor must not move when 'sol' is commanded at the start of the "
        "line.");

    /* eol->sol should move to start of line */
    cursor_sol(&c);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(conf.lineOffset, c.curLine,
        "Cursor must not line-hop when 'sol' is commanded.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(conf.colOffset, c.curCol,
        "Cursor must move to start of line when 'sol' is commanded.");

    /* When hanging, eol should move to end of line */
    c.curCol = 999;
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_eol(&c, l),
        "Cursor must be in bounds if line is short when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(conf.lineOffset, c.curLine,
        "Cursor must not line-hop when 'eol' is commanded.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(l->len + conf.colOffset, c.curCol,
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
    init_cursor(&c, conf.lineOffset, conf.colOffset, conf.lineOffset,
                conf.colOffset);
    update_cursor_max_bounds(&c, conf.lineOffset + 1, conf.colOffset + 1);
    TEST_ASSERT_GREATER_THAN(conf.lineOffset + 1, l->len);  /* Test setup */

    /* eol should move us out of bounds */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, cursor_eol(&c, l),
        "Cursor is out of bounds when 'eol' is commanded if line is too long "
        "to handle.");
}

/* Placing the cursor out of bounds by moving the upper bounds. */
void test_update_oob(void)
{
    struct Cursor c;
    init_cursor(&c, 6, 7, 5, 6);  /* Weird lower bounds and initial value */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, update_cursor_max_bounds(&c, 6, 7),
        "Update must return 0 if the cursor is still in-bounds.");

    /* An oob update */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, update_cursor_max_bounds(&c, 1, 1),
        "Update must return 1 if the cursor is now out of bounds.");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_elementary_cursor_motion);
    RUN_TEST(test_elementary_cursor_oob);
    RUN_TEST(test_line_cursor_motion);
    RUN_TEST(test_line_cursor_oob);
    RUN_TEST(test_update_oob);
    return UNITY_END();
}
