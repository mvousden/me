/* Needed for in-memory file buffers. */
#define _GNU_SOURCE

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "conf.h"
#include "unity.h"

extern struct MeConf conf;

#define BIG_ENOUGH 1024 * conf.openChunkSize * 3

struct Buffer* testBuffer = NULL;
FILE* memFile = NULL;
char* result;  /* Should be big enough */

/* Creating and initialising an empty buffer, opening an in-memory file, and
 * creating and zeroing the test character array. */
void setUp(void)
{
    testBuffer = malloc(sizeof(struct Buffer));
    TEST_ASSERT_NOT_NULL_MESSAGE(testBuffer, "OOM");
    init_buffer(testBuffer);
    memFile = fmemopen(NULL, BIG_ENOUGH, "w+");
    result = calloc(BIG_ENOUGH, sizeof(char));
    TEST_ASSERT_NOT_NULL_MESSAGE(result, "OOM");
}

void tearDown(void)
{
    if (testBuffer)
    {
        destroy_buffer_content(testBuffer);
        free(testBuffer);
        testBuffer = NULL;
    }
    if (memFile)
    {
        fclose(memFile);
        memFile = NULL;
    }
    if (result)
    {
        free(result);
        result = NULL;
    }
}

/* Writes buffer content to str. Each line's content is split by \n. str better
 * be big enough. */
void buffer_to_string(struct Buffer const* const buffer, char* str)
{
    struct Line* currentLine = buffer->topLine;
    do
    {
        str = stpcpy(str, currentLine->content);
        currentLine = currentLine->next;
        if (!currentLine) break;
        *str++ = '\n';
    }
    while (1);
}

void test_empty_file(void)
{
    /* Nothing to write to memory file. We just go. */
    populate_buffer_from_file(testBuffer, memFile);
    buffer_to_string(testBuffer, result);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", result,
        "Loading an empty file should result in an empty buffer.");
}

void test_writable_oneline(void)
{
    char* cp = \
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int len = fprintf(memFile, "%s", cp);
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen(cp), len,
        "Testing error: string not written to memory file correctly.");
    rewind(memFile);
    populate_buffer_from_file(testBuffer, memFile);
    buffer_to_string(testBuffer, result);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(cp, result,
        "Loading a single line of writable characters should result in a "
        "single line containing those characters in the same sequence.");
}

void test_nonwritable(void)
{
    /* Creating a character array with all possible non-printable character
     * values except 0, followed by a space and a 0. */
    char cp[256] = {0};
    size_t ix = 0;
    for (char c = CHAR_MIN; c < 0x21; c++)
    {
        if (c && c != '\n')
        {
            cp[ix] = c;
            ix++;
        }
    }
    int len = fprintf(memFile, "%s", cp);
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen(cp), len,
        "Testing error: string not written to memory file correctly.");
    rewind(memFile);
    populate_buffer_from_file(testBuffer, memFile);
    buffer_to_string(testBuffer, result);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(" ", result,
        "Non-writable characters should be ignored when loaded.");
}

void test_newlines(void)
{
    char* cp = "\n\n";
    int len = fprintf(memFile, "%s", cp);
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen(cp), len,
        "Testing error: string not written to memory file correctly.");
    rewind(memFile);
    populate_buffer_from_file(testBuffer, memFile);
    buffer_to_string(testBuffer, result);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(cp, result,
        "Loading a file with two newlines should result in a buffer with "
        "three empty lines.");
}

void test_multiline_lipsum(void)
{
    char* cp = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed\n"
        "do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut\n"
        "enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
        "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in\n"
        "reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla\n"
        "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in\n"
        "culpa qui officia deserunt mollit anim id est laborum.";
    int len = fprintf(memFile, "%s", cp);
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen(cp), len,
        "Testing error: string not written to memory file correctly.");
    rewind(memFile);
    populate_buffer_from_file(testBuffer, memFile);
    buffer_to_string(testBuffer, result);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(cp, result,
        "Loading a file with two newlines should result in a buffer with "
        "three empty lines.");
}

void test_line_memory_expansion(void)
{
    /* Writes a number of 'I's equal to twice the size of the buffer plus
     * one. */
    size_t ix;
    for (ix = 0; ix < conf.openChunkSize * 2 + 1; ix++) fputc('I', memFile);
    fputc(0, memFile);  /* +1 */
    rewind(memFile);
    populate_buffer_from_file(testBuffer, memFile);
    buffer_to_string(testBuffer, result);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(conf.openChunkSize * 2 + 2, strlen(result),
        "The memory allocated to a line must expand/distend with content.");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_empty_file);
    RUN_TEST(test_writable_oneline);
    RUN_TEST(test_nonwritable);
    RUN_TEST(test_newlines);
    RUN_TEST(test_multiline_lipsum);
    RUN_TEST(test_line_memory_expansion);
    return UNITY_END();
}

#undef BIG_ENOUGH
