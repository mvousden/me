#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "helpers.h"
#include "line.h"
#include "unity.h"

#define ALPHABET_LEN 26
#define CHAIN_LEN 3  /* Greater than one */

extern struct MeConf conf;

struct Line* testLine = NULL;
struct Line* lineChain[CHAIN_LEN] = {NULL};

/* A convenient test string that is easy to debug. */
const char* const alphabet = "abcdefghijklmnopqrstuvwxyz";

/* Lines get heaped in me, makes cleanup consistent between test and
 * source. Each test gets three lines to work with, whether or not they want
 * them. */
void setUp(void)
{
    testLine = malloc(sizeof(struct Line));
    TEST_ASSERT_NOT_NULL_MESSAGE(testLine, "OOM");
    init_line(testLine, NULL, NULL);
}

/* Not called by unity explicitly - tests call this if needed. Initialises and
 * connects lines like 0 --> 1 --> 2 --> ... --> CHAIN_LEN-1 */
void setUpMoreLines(void)
{
    size_t index;
    for (index = 0; index < CHAIN_LEN; index++)
    {
        lineChain[index] = malloc(sizeof(struct Line));
        TEST_ASSERT_NOT_NULL_MESSAGE(lineChain[index], "OOM");
    }
    for (index = 0; index < CHAIN_LEN; index++)
        init_line(lineChain[index],
                  index ? lineChain[index - 1] : NULL,
                  index != CHAIN_LEN - 1 ? lineChain[index + 1] : NULL);
}

/* Tests cascading line destruction, somewhat. */
void tearDown(void)
{
    if (testLine)
    {
        destroy_line_cascade(testLine);
        testLine = NULL;
    }
    if (lineChain[0])
    {
        destroy_line_cascade(lineChain[0]);
        lineChain[0] = NULL;
    }
}

void test_append_char_single(void)
{
    append_char(testLine, 'a');
    TEST_ASSERT_EQUAL_UINT_MESSAGE(1, testLine->len,
        "Lines with a single character must have a length of one.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("a", testLine->content,
        "Line contains something other than appended character.");
}

/* Test that append_char works when the line's buffer is full - should expand
 * the buffer silently. No way to test the expansion itself simply. */
void test_append_char_multiple_buffer_expansion(void)
{
    const size_t finalSize = conf.initLineSize * 3 + 3;
    char* comparison = malloc(sizeof(char) * finalSize);
    char* c;
    TEST_ASSERT_NOT_NULL_MESSAGE(comparison, "OOM");
    for (c = comparison + finalSize - 1; c != comparison; *c-- = 0);  /* Dn */
    while (comparison != c - (finalSize - 1))
    {
        append_char(testLine, ';');
        *c++ = ';';  /* Up */
        *c = 0;
        TEST_ASSERT_EQUAL_STRING_MESSAGE(comparison, testLine->content,
            "Line missing appended character.");
    }
    free(comparison);
}

void test_append_string_empty(void)
{
    const char* c;
    /* Appending alphabet by char */
    for (c = alphabet; *c; append_char(testLine, *c++));
    /* Appending an empty string */
    append_string(testLine, "");
    TEST_ASSERT_EQUAL_STRING_MESSAGE(alphabet, testLine->content,
        "Appending an empty string must not affect line content.");
}

void test_append_string_short(void)
{
    const char* c;
    /* Appending half of alphabet by char */
    for (c = alphabet; c < alphabet + 13; append_char(testLine, *c++));
    /* Appending the other half by string */
    append_string(testLine, c);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(alphabet, testLine->content,
        "Appending a string should concatenate.");
}

void test_append_string_buffer_expansion(void)
{
    /* How many 'alphabets' do we need to force the line buffer to expand. */
    size_t numAlphabets = conf.initLineSize / strlen(alphabet) + 1;
    char* const comparison = calloc(strlen(alphabet) * numAlphabets + 1,
                                    sizeof(char));
    char* comparisonEnd = comparison;
    while (numAlphabets)
    {
        append_string(testLine, alphabet);
        comparisonEnd = slide_copy(alphabet, comparisonEnd);
        numAlphabets--;
    }
    TEST_ASSERT_EQUAL_STRING_MESSAGE(comparison, testLine->content,
        "Appending many strings must expand the content buffer.");
    free(comparison);
}

/* Content should be zero-initialised. */
void test_content_buffer_initialised_to_zero(void)
{
    char* c;
    for (c = testLine->content; c < testLine->content + conf.initLineSize;
         TEST_ASSERT_EQUAL_CHAR_MESSAGE(0, *c++,
             "Line content must be zero-initialised."));
}

/* New content should be zero-initialised. */
void test_content_buffer_initialised_to_zero_when_expanded(void)
{
    char* c;
    /* Insert enough characters to trigger an expansion */
    while (testLine->len < conf.initLineSize) append_char(testLine, '!');
    /* Expanded content should be filled with zeroes */
    for (c = testLine->content; c < testLine->content + 2 * conf.initLineSize;
         c++)
        TEST_ASSERT_EQUAL_CHAR_MESSAGE(
            c < testLine->content + conf.initLineSize ? '!' : 0, *c,
            "Expanded line content must be zero-initialised.");
}

void test_delete_char_from_line_at_zero(void)
{
    append_char(testLine, '+');
    append_char(testLine, '-');
    delete_char_from_line(testLine, 0);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("-", testLine->content,
        "Deleting at low offsets should just delete earliest char.");
}

void test_delete_char_from_line_empty(void)
{
    delete_char_from_line(testLine, 0);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", testLine->content,
        "Delete on an empty line should stay empty.");
}

void test_delete_char_from_line_full_end(void)
{
    size_t index;
    for (index = 0; index != ALPHABET_LEN;
         append_char(testLine, alphabet[index++]));
    delete_char_from_line(testLine, 99);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(
        "abcdefghijklmnopqrstuvwxyz", testLine->content,
        "Delete past end of line should have no effect.");
}

void test_delete_char_from_line_repeat(void)
{
    size_t index;
    for (index = 0; index != ALPHABET_LEN;
         append_char(testLine, alphabet[index++]));
    for (index = 2; index != ALPHABET_LEN; index++)
        delete_char_from_line(testLine, 1);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("az" , testLine->content,
        "Repeated deletions of the same location should stack.");
}

void test_delete_char_from_line_single(void)
{
    size_t index;
    for (index = 0; index != ALPHABET_LEN;
         append_char(testLine, alphabet[index++]));
    delete_char_from_line(testLine, 15);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(
        "abcdefghijklmnoqrstuvwxyz" , testLine->content,
        "Line deletion expected to move 15th letter 'p', but didn't.");
}

/* Not really testing anything, but the memory checker will yell if this
 * doesn't work - uncoupling the last two lines in the lineTrio from the first
 * line, then destroying the last two lines individually without cascading
 * should be free from memory leaks */
void test_destroy_line(void)
{
    setUpMoreLines();
    lineChain[0]->next = NULL;
    destroy_line(lineChain[1]);
    destroy_line(lineChain[2]);
}

/* Not really testing anything, but the memory checker will yell if this
 * doesn't work - uncoupling the last two lines in the lineChain trio from the
 * first line, and cascade-destroying the middle line only should be free from
 * memory leaks */
void test_destroy_line_cascade(void)
{
    setUpMoreLines();
    lineChain[0]->next = NULL;
    destroy_line_cascade(lineChain[1]);
}

void test_init_line_empty(void)
{
    /* Empty line initialises as expected. */
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, testLine->len,
        "Lines must have an initial length of zero.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", testLine->content,
        "Lines must start with 0.");
    TEST_ASSERT_NULL_MESSAGE(testLine->prev,
        "Lines are initialised with no neighbours, unless passed as args.");
    TEST_ASSERT_NULL_MESSAGE(testLine->next,
        "Lines are initialised with no neighbours, unless passed as args.");
}

void test_init_line_chained(void)
{
    size_t index;
    setUpMoreLines();

    for (index = 0; index < CHAIN_LEN; index++)
    {
        if (!index)
            TEST_ASSERT_NULL_MESSAGE(lineChain[index]->prev,
                "Lines are initialised with no neighbours by default.");
        else
            TEST_ASSERT_EQUAL_HEX64_MESSAGE(lineChain[index - 1],
                                            lineChain[index]->prev,
                "Line 'prev' relationship failed.");
        if (index == CHAIN_LEN - 1)
            TEST_ASSERT_NULL_MESSAGE(lineChain[index]->next,
                "Lines are initialised with no neighbours by default.");
        else
            TEST_ASSERT_EQUAL_HEX64_MESSAGE(lineChain[index + 1],
                                            lineChain[index]->next,
                "Line 'next' relationship failed.");
    }
}

void test_insert_char_into_line_append(void)
{
    insert_char_into_line(testLine, alphabet[0], ALPHABET_LEN);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("a", testLine->content,
        "Characters should be appended when offset is > length.");
}

/* This "should never happen", as the current column is restricted by the
 * configurator. */
void test_insert_char_into_line_at_zero(void)
{
    insert_char_into_line(testLine, '-', 0);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("-", testLine->content,
        "Characters should be pre-pended when offset is zero.");
    insert_char_into_line(testLine, '|', 0);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("|-", testLine->content,
        "Characters should be pre-pended when offset is zero.");
}

void test_insert_char_into_line_empty(void)
{
    insert_char_into_line(testLine, alphabet[0], 0);  /* "a" */
    insert_char_into_line(testLine, alphabet[1], 0);  /* "ba" */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ba", testLine->content,
        "Characters should be inserted when offset is <= length.");
}

/* Should append if offset is too big - half of these chars are appended, half
 * inserted */
void test_insert_char_into_line_mixed_mode(void)
{
    size_t index;
    for (index = 0; index != ALPHABET_LEN;
         insert_char_into_line(testLine, alphabet[index++], 13));
    TEST_ASSERT_EQUAL_STRING_MESSAGE(
        "abcdefghijklmzyxwvutsrqpon", testLine->content,
        "Characters should be appended when offset is greater than length.");
}

/* Null terminator shouldn't stop insertion - niche */
void test_insert_char_into_line_null_case(void)
{
    insert_char_into_line(testLine, alphabet[0], 0);  /* 'a', 0 */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("a", testLine->content,
        "Empty insertion should append.");
    insert_char_into_line(testLine, 0, 0);            /* 0, 'a', 0 */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", testLine->content,
        "Insertion should respect hidden NULL characters.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("a", testLine->content + 1,
        "Insertion should respect hidden NULL characters.");
    insert_char_into_line(testLine, alphabet[1], 2);  /* 0, 'a', 'b', 0 */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", testLine->content,
        "Insertion should respect hidden NULL characters.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ab", testLine->content + 1,
        "Insertion should respect hidden NULL characters.");
    delete_char_from_line(testLine, 0);               /* 'a', 'b', '0' */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ab", testLine->content,
        "Insertion should respect hidden NULL characters.");
}

void test_merge_line_with_next_null(void)
{
    TEST_ASSERT_NULL_MESSAGE(testLine->next,
        "Test precondition violated - this line should be isolated.");
    merge_line_with_next(testLine);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", testLine->content,
        "Merging two empty lines should remain empty.");
    TEST_ASSERT_NULL_MESSAGE(testLine->next,
        "Line should have no successor.");
}

void test_merge_line_with_next_content_content(void)
{
    size_t index;
    const char* c = alphabet;
    setUpMoreLines();

    for (index = 0; index < 2; index++)
        do append_char(lineChain[index], *c++);
        while (lineChain[index]->len < 3);
    merge_line_with_next(lineChain[0]);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("abcdef", lineChain[0]->content,
        "Line concatenation failure.");
}

void test_merge_line_with_next_content_empty(void)
{
    const char* c = alphabet;
    setUpMoreLines();
    do append_char(lineChain[0], *c++);
    while (lineChain[0]->len < 2);
    merge_line_with_next(lineChain[0]);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ab", lineChain[0]->content,
        "Line concatenation failure.");
}

void test_merge_line_with_next_empty_content(void)
{
    const char* c = alphabet;
    setUpMoreLines();
    do append_char(lineChain[1], *c++);
    while (lineChain[1]->len < 7);
    merge_line_with_next(lineChain[0]);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("abcdefg", lineChain[0]->content,
        "Line concatenation failure.");
}

void test_merge_line_with_next_empty_empty(void)
{
    size_t index;
    setUpMoreLines();

    for (index = 0; index < CHAIN_LEN; index++)
    {
        merge_line_with_next(lineChain[0]);
        TEST_ASSERT_EQUAL_STRING_MESSAGE("", lineChain[0]->content,
            "Merging two empty lines should remain empty.");
    }
    TEST_ASSERT_NULL_MESSAGE(lineChain[0]->next,
        "Line should have no successor after repeated collapse.");
}

void test_split_line_empty(void)
{
    TEST_ASSERT_NULL_MESSAGE(testLine->next, "Test precondition failure.");
    split_line(testLine, 0, 0, 0);
    TEST_ASSERT_NOT_NULL_MESSAGE(testLine->next,
        "Line should have a 'next' defined after splitting.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", testLine->content,
        "Splitting an empty line should leave it empty.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", testLine->next->content,
        "Splitting an empty line should create an empty 'next', if no "
        "whitespace argument is passed.");
}

void test_split_line_content(void)
{
    const char* c = alphabet;
    do append_char(testLine, *c++);
    while (testLine->len < 6);
    split_line(testLine, 3, 0, 0);
    TEST_ASSERT_NOT_NULL_MESSAGE(testLine->next,
        "Line should have a 'next' defined after splitting.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("abc", testLine->content,
        "Lines should be split at the offset.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("def", testLine->next->content,
        "Lines should be split at the offset.");
}

/* Split chain at zero, test the new line is inserted in the 'linked'
 * structure */
void test_split_line_respects_chain(void)
{
    const struct Line* curLine;
    setUpMoreLines();
    split_line(lineChain[0], 0, 0, 0);
    for (curLine = lineChain[0]; curLine != lineChain[CHAIN_LEN - 1];
         curLine = curLine->next)
        TEST_ASSERT_NOT_NULL_MESSAGE(curLine->next,
            "Line should have a 'next' defined after splitting.");
    TEST_ASSERT_NULL_MESSAGE(curLine->next,
        "Final line in chain should have no 'next' defined after splitting.");
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(curLine, lineChain[CHAIN_LEN - 1],
        "End of chain should remain consistent after splitting.");
}

/* Various whitespace-adding checks */
void test_split_line_whitespace(void)
{
    struct Line* curLine;
    split_line(testLine, 0, ' ', 0);
    curLine = testLine->next;
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", curLine->content,
        "No whitespace should be added to split line if wsCount is 0.");
    split_line(curLine, 0, ' ', 1);
    curLine = curLine->next;
    TEST_ASSERT_EQUAL_STRING_MESSAGE(" ", curLine->content,
        "One whitespace should be added to split line if wsCount is one.");
    split_line(curLine, 1, ' ', 1);
    curLine = curLine->next;
    TEST_ASSERT_EQUAL_STRING_MESSAGE(" ", curLine->content,
        "One whitespace should be added to split line if wsCount is one, "
        "working with a non-zero offset.");
    split_line(curLine, 1, 'a', 4);
    curLine = curLine->next;
    TEST_ASSERT_EQUAL_STRING_MESSAGE("aaaa", curLine->content,
        "Non-whitespace characters should also be compatible.");
    split_line(curLine, 2, 'b', 2);
    curLine = curLine->next;
    TEST_ASSERT_EQUAL_STRING_MESSAGE("bbaa", curLine->content,
        "Whitespace must be pre-pended.");
    split_line(curLine, 1, '\t', 4);
    curLine = curLine->next;
    TEST_ASSERT_EQUAL_STRING_MESSAGE("\t\t\t\tbaa", curLine->content,
        "Non-printable characters should also be written.");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_append_char_single);
    RUN_TEST(test_append_char_multiple_buffer_expansion);
    RUN_TEST(test_append_string_empty);
    RUN_TEST(test_append_string_short);
    RUN_TEST(test_append_string_buffer_expansion);
    RUN_TEST(test_content_buffer_initialised_to_zero);
    RUN_TEST(test_content_buffer_initialised_to_zero_when_expanded);
    RUN_TEST(test_delete_char_from_line_at_zero);
    RUN_TEST(test_delete_char_from_line_empty);
    RUN_TEST(test_delete_char_from_line_full_end);
    RUN_TEST(test_delete_char_from_line_repeat);
    RUN_TEST(test_delete_char_from_line_single);
    RUN_TEST(test_destroy_line);
    RUN_TEST(test_destroy_line_cascade);
    RUN_TEST(test_init_line_empty);
    RUN_TEST(test_init_line_chained);
    RUN_TEST(test_insert_char_into_line_append);
    RUN_TEST(test_insert_char_into_line_at_zero);
    RUN_TEST(test_insert_char_into_line_mixed_mode);
    RUN_TEST(test_insert_char_into_line_null_case);
    RUN_TEST(test_merge_line_with_next_null);
    RUN_TEST(test_merge_line_with_next_content_content);
    RUN_TEST(test_merge_line_with_next_content_empty);
    RUN_TEST(test_merge_line_with_next_empty_content);
    RUN_TEST(test_merge_line_with_next_empty_empty);
    RUN_TEST(test_split_line_empty);
    RUN_TEST(test_split_line_content);
    RUN_TEST(test_split_line_respects_chain);
    RUN_TEST(test_split_line_whitespace);
    return UNITY_END();
}

#undef ALPHABET_LEN
#undef CHAIN_LEN
