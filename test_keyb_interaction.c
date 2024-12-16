/* Not very low-level tests, but comparing a sequence of keyboard inputs to a
 * desired state. */

#include <limits.h>
#include <stdlib.h>

#include "keyb.h"
#include "keyb_defs.h"
#include "state.h"
#include "unity.h"

extern struct MeState state;

void setUp(void)
{
    init_state(NULL);
    update_cursor_max_bounds(&state.cursor, SHRT_MAX, SHRT_MAX);
}

void tearDown(void){destroy_state();}

const unsigned caseCharDeletionSetup[] = {'1', '2', CTRL_('m'),
    '3', ALT_('<'), 0};
void test_char_deletion(void)
{
    const unsigned delSynonyms[] = {DEL, CTRL_('d')};
    const size_t numDelSynonyms = 2;
    size_t index;
    const unsigned* restrict u;

    /* Repeated deletion using DEL and C-d. */
    for (index = 0; index < numDelSynonyms; index++)
    {
        /* Type the chars. */
        u = caseCharDeletionSetup;
        while (*u) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(*u++),
            "All commands in this test should return 1.");

        /* Do some deletions and test what comes back */
        TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(delSynonyms[index]),
            "All commands in this test should return 1.");
        TEST_ASSERT_EQUAL_STRING_MESSAGE("2", state.buffer.topLine->content,
            "Deletion must operate on value under cursor.");
        TEST_ASSERT_EQUAL_STRING_MESSAGE("3",
            state.buffer.topLine->next->content,
            "Deletion in-line must not affect other lines.");

        TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(delSynonyms[index]),
            "All commands in this test should return 1.");
        TEST_ASSERT_EQUAL_STRING_MESSAGE("", state.buffer.topLine->content,
            "Deletion must be able to create empty lines.");
        TEST_ASSERT_EQUAL_STRING_MESSAGE("3",
            state.buffer.topLine->next->content,
            "Deletion in-line must not affect other lines.");

        TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(delSynonyms[index]),
            "All commands in this test should return 1.");
        TEST_ASSERT_EQUAL_STRING_MESSAGE("3", state.buffer.topLine->content,
            "Deletion must be able to merge lines.");
        TEST_ASSERT_NULL_MESSAGE(state.buffer.topLine->next,
            "Deletion must remove merged lines.");

        TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(delSynonyms[index]),
            "All commands in this test should return 1.");
        TEST_ASSERT_EQUAL_STRING_MESSAGE("", state.buffer.topLine->content,
            "Deletion must be able to empty the document.");
        TEST_ASSERT_NULL_MESSAGE(state.buffer.topLine->next,
            "Deletion must not add lines.");

        TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(delSynonyms[index]),
            "All commands in this test should return 1.");
        TEST_ASSERT_EQUAL_STRING_MESSAGE("", state.buffer.topLine->content,
            "Repeated deletions should have no effect once buffer is empty.");
        TEST_ASSERT_NULL_MESSAGE(state.buffer.topLine->next,
            "Deletion must not add lines.");
    }

    /* Backspace testing */
    /* Type the chars. */
    u = caseCharDeletionSetup;
    while (*u) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(*u++),
        "All commands in this test should return 1.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(BACKSPACE),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("12", state.buffer.topLine->content,
        "Backspace at start of buffer must make no change to current line.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("3", state.buffer.topLine->next->content,
        "Backspace at start of buffer must make no change to any line.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('>')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(BACKSPACE),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("12", state.buffer.topLine->content,
        "Backspace in another line must make no change other lines.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", state.buffer.topLine->next->content,
        "Backspace must delete a character when used within a line.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)'3'),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ARR_LF),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(BACKSPACE),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("123", state.buffer.topLine->content,
        "Backspace must merge lines when deleting at the start of a line.");
    TEST_ASSERT_NULL_MESSAGE(state.buffer.topLine->next,
        "Backspace must remove merged lines.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(BACKSPACE),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("13", state.buffer.topLine->content,
        "Backspace must remove characters within a line when used.");
}

const char caseCharMoveSetup[] = {'a', 'b', 'c', CTRL_('m'),
    'd', 'e', 'f', CTRL_('m'),
    'g', 'h', 'i', 0};
void test_char_movement(void)
{
    /* Type the chars. */
    const char* restrict c = caseCharMoveSetup;
    while (*c) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)*c++),
        "All commands in this test should return 1.");

    /* Line home and end, with repeats */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)NAV_HOME),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "nav_home column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "nav_home line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)NAV_HOME),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "nav_home column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "nav_home line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)NAV_END),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "nav_end column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "nav_end line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)NAV_END),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "nav_end column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "nav_end line test failed.");

    /* CTRL-equivalents to nav_home and nav_end */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('a')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "C-a column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-a line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('a')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "C-a column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-a line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('e')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "C-e column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-e line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('e')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "C-e column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-e line test failed.");

    /* Doc home and doc end in the same way, with repeats. */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ALT_('<')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "M-< column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "M-< line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ALT_('<')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "M-< column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "M-< line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ALT_('>')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "M-> column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "M-> line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ALT_('>')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "M-> column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "M-> line test failed.");

    /* Arrow keys */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ARR_LF),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curCol,
        "Left-arrow column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "Left-arrow line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ARR_RT),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "Right-arrow column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "Right-arrow line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ARR_UP),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "Up-arrow column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curLine,
        "Up-arrow line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ARR_DN),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "Down-arrow column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "Down-arrow line test failed.");

    /* Arrow key edge cases */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)NAV_HOME),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ARR_LF),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "Left-arrow column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curLine,
        "Left-arrow line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ARR_RT),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "Right-arrow column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "Right-arrow line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ARR_DN),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "Down-arrow column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "Down-arrow line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ALT_('<')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ARR_UP),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "Up-arrow column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Up-arrow line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ALT_('>')),
        "All commands in this test should return 1.");
    /* CTRL-equivalents to arrow keys */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curCol,
        "C-b column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-b line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "C-f column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-f line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('p')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "C-p column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curLine,
        "C-p line test failed.");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('n')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "C-n column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-n line test failed.");

    /* CTRL-equivalent edge cases */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)NAV_HOME),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, state.cursor.curCol,
        "C-b column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curLine,
        "C-b line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "C-f column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-f line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('n')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "C-n column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curLine,
        "C-n line test failed.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)ALT_('<')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)CTRL_('p')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "C-p column test failed.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "C-p line test failed.");
}

const char* const caseWordMoveSetupTp = " hello\"hello@hello1he(llo()()";
const char* const caseWordMoveSetupBt = "abcdefghijklmnopqrstuvwxyz1234567890";
void test_word_movement(void)
{
    /* Type the chars, and move the cursor to top of buffer. */
    const char* restrict c = caseWordMoveSetupTp;
    while (*c) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)*c++),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('m')),
        "All commands in this test should return 1.");
    c = caseWordMoveSetupBt;
    while (*c) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)*c++),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('m')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key('('),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('m')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('m')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key('a'),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('m')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(')'),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('<')),
        "All commands in this test should return 1.");

    /* Hop forward and check each jump. */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(6, state.cursor.curCol,
        "Forward hop 1 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Forward hop 1 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(12, state.cursor.curCol,
        "Forward hop 2 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Forward hop 2 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(21, state.cursor.curCol,
        "Forward hop 3 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Forward hop 3 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(25, state.cursor.curCol,
        "Forward hop 4 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Forward hop 4 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(36, state.cursor.curCol,
        "Forward hop 5 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curLine,
        "Forward hop 5 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curCol,
        "Forward hop 6 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(4, state.cursor.curLine,
        "Forward hop 6 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('f')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curCol,
        "Forward hop 7 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(5, state.cursor.curLine,
        "Forward hop 7 failed (line).");

    /* Hop backward and check each jump. */
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "Backward hop 1 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(4, state.cursor.curLine,
        "Backward hop 1 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "Backward hop 2 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curLine,
        "Backward hop 2 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(22, state.cursor.curCol,
        "Backward hop 3 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Backward hop 3 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(13, state.cursor.curCol,
        "Backward hop 4 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Backward hop 4 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(7, state.cursor.curCol,
        "Backward hop 5 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Backward hop 5 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curCol,
        "Backward hop 6 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Backward hop 6 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "Backward hop 7 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Backward hop 7 failed (line).");

    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(ALT_('b')),
        "All commands in this test should return 1.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curCol,
        "Backward hop 8 failed (col).");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, state.cursor.curLine,
        "Backward hop 8 failed (line).");

    /* For sanity, test top line hasn't been modified. */
    TEST_ASSERT_EQUAL_STRING_MESSAGE(caseWordMoveSetupTp,
                                     state.buffer.topLine->content,
        "Word hopping must not alter line content.");
}

const unsigned caseHanging[] = {'a', 'b', 'c', CTRL_('m'), 'd',
    ARR_UP, NAV_END, ARR_DN, 'e', 0};
void test_hanging_cursor(void)
{
    /* Type the chars. */
    const unsigned* restrict u = caseHanging;
    while (*u) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(*u++),
        "All commands in this test should return 1.");

    /* Check line content */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("abc",
                                     state.buffer.topLine->content,
        "Top line should remain unaffected by a hanging cursor.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("de",
                                     state.buffer.topLine->next->content,
        "Bottom line should remain whole after hanging cursor is used.");

    /* Check cursor position */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, state.cursor.curLine,
        "Hanging cursor should end on bottom line when used.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, state.cursor.curCol,
        "Hanging cursor must be at the end of the line when used.");
}

const char caseLineBreakInput[] = {'a', 'b', 'c', CTRL_('m'),
    'd', 'e', 'f', 0};
const char* const caseLineBreakFirst = "abc";
const char* const caseLineBreakSecond = "def";
void test_line_break(void)
{
    /* Type the chars. */
    const char* restrict c = caseLineBreakInput;
    while (*c) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)*c++),
        "All commands in this test should return 1.");

    /* Compare the lines we've written, and state 'geometry'. */
    TEST_ASSERT_EQUAL_STRING_MESSAGE(caseLineBreakFirst,
                                     state.buffer.topLine->content,
        "First line should contain all characters before the break.");
    TEST_ASSERT_NOT_NULL_MESSAGE(state.buffer.topLine->next,
        "There should be at least two lines.");
    TEST_ASSERT_NULL_MESSAGE(state.buffer.topLine->next->next,
        "There should not be three lines.");
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(state.buffer.topLine->next,
                                    state.buffer.currentLine,
        "The current line should be the second line in the state.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE(caseLineBreakSecond,
                                     state.buffer.topLine->next->content,
        "Second line should contain all characters after the break.");
}

const char* const caseVisibleASCII = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEF"
"GHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
void test_visible_chars(void)
{
    /* Type the chars. */
    const char* restrict c = caseVisibleASCII;
    while (*c) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)*c++),
        "All commands in this test should return 1.");

    /* Compare the line we've written. */
    TEST_ASSERT_EQUAL_STRING_MESSAGE(caseVisibleASCII,
                                     state.buffer.currentLine->content,
        "All of these characters should be type-able and visible natively.");

    /* No other lines should exist. */
    TEST_ASSERT_NULL_MESSAGE(state.buffer.topLine->next,
        "There should be no more than one line.");
    TEST_ASSERT_NULL_MESSAGE(state.buffer.topLine->prev,
        "There should be no more than one line.");
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(state.buffer.topLine,
                                    state.buffer.currentLine,
        "The current line should be the top line in the state.");
}

const int caseWhitespaceZap[] = {' ', 'a', CTRL_('m'),
    ' ', ' ', 'b', CTRL_('m'),
    'c', CTRL_('m'),
    'd', ' ', CTRL_('m'),
    'e', ' ', CTRL_('m'),
    'a', 'p', 'p', ' ', ' ', ' ', 'l', 'e', ' ',
    ALT_('<'),
    ALT_('\\'), CTRL_('n'),
    ALT_('\\'), CTRL_('n'),
    ALT_('\\'), CTRL_('n'),
    ALT_('\\'), CTRL_('n'),
    CTRL_('f'), ALT_('\\'), CTRL_('n'),
    ALT_('f'), ALT_('\\'),
    ALT_('<'), 0};
void test_whitespace_zap(void)
{
    /* Type the chars. */
    const int* restrict c = caseWhitespaceZap;
    while (*c) TEST_ASSERT_EQUAL_MESSAGE(1, proc_key((unsigned)*c++),
        "All commands in this test should return 1.");

    /* Check all lines with what we expect. */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("a",
                                     state.buffer.currentLine->content,
        "Alt-\\ should trim whitespace from the cursor.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('n')),
        "All commands in this test should return 1.");

    TEST_ASSERT_EQUAL_STRING_MESSAGE("b",
                                     state.buffer.currentLine->content,
        "Alt-\\ should trim all whitespace from the cursor.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('n')),
        "All commands in this test should return 1.");

    TEST_ASSERT_EQUAL_STRING_MESSAGE("c",
                                     state.buffer.currentLine->content,
        "Alt-\\ should do nothing when there is no whitespace to trim.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('n')),
        "All commands in this test should return 1.");

    TEST_ASSERT_EQUAL_STRING_MESSAGE("d ",
                                     state.buffer.currentLine->content,
        "Alt-\\ should leave whitespace after the trim.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('n')),
        "All commands in this test should return 1.");

    TEST_ASSERT_EQUAL_STRING_MESSAGE("e",
                                     state.buffer.currentLine->content,
        "Alt-\\ should trim even when not at the start of a line.");
    TEST_ASSERT_EQUAL_MESSAGE(1, proc_key(CTRL_('n')),
        "All commands in this test should return 1.");

    TEST_ASSERT_EQUAL_STRING_MESSAGE("apple ",
                                     state.buffer.currentLine->content,
        "Alt-\\ should trim multiple spaces.");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_line_break);
    RUN_TEST(test_visible_chars);
    RUN_TEST(test_char_movement);
    RUN_TEST(test_word_movement);
    RUN_TEST(test_char_deletion);
    RUN_TEST(test_hanging_cursor);
    RUN_TEST(test_whitespace_zap);
    return UNITY_END();
}
