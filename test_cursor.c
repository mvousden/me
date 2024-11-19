/* Somewhat redundant with the keyboard tests, but more introspective. */

#include <limits.h>

#include "cursor.h"
#include "unity.h"

void setUp(void){}
void tearDown(void){}

/* Do the cursor move and update methods return what they should? oob =
 * out-of-bounds. */
void test_cursor_oob(void)
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

    /* Expected (0) cases */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_up(&c),
        "'Up' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_dn(&c),
        "'Down' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_lt(&c),
        "'Left' cursor movement must return 0 when not out of bounds.");
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, cursor_rt(&c),
        "'Right' cursor movement must return 0 when not out of bounds.");

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

    /* An oob update */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, update_cursor_max_bounds(&c, 1, 1),
        "Update must return 1 if the cursor is now out of bounds.");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cursor_oob);
    return UNITY_END();
}
