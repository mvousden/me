#include <stdlib.h>

#include <string.h>

#include "helpers.h"
#include "unity.h"

#define SRC_SIZ 10
#define BUF_SIZ 2 * SRC_SIZ

char buf[BUF_SIZ];
char src[SRC_SIZ];

/* Buffers! */
void setUp(void)
{
    size_t index;
    for (index = 0; index < BUF_SIZ; buf[index++] = 0);
    for (index = 0; index < SRC_SIZ; src[index++] = 0);
}

void tearDown(void){}

/* Oh God how horrifying */
void test_is_alphanum(void)
{
    /* The last delimiter (0x80) denotes the end of ASCII. */
    const unsigned char delimiters[] = {'0', ':', 'A', '[', 'a', '{', 0x80, 0};
    const int answers[]              = {0  , 1  , 0  , 1  , 0  , 1  , 0};
    const unsigned rangeMax = strlen((char*)delimiters);
    unsigned short range;
    unsigned char test = 0;
    for (range = 0; range < rangeMax; range++)
        for (; test < delimiters[range];
             TEST_ASSERT_EQUAL_UINT(answers[range], is_alphanum((char)test++)));
}

void test_slide_copy_empty(void)
{
    char* c = slide_copy(src, buf);
    *c = 0;  /* Unnecessary: Just to really hammer home the point. */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", src, "Source must remain empty.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", buf,
        "Buf should be empty by default.");
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(c, buf,
        "If no characters written, return value should be the same as buf.");
}

void test_slide_copy_empty_buf(void)
{
    src[0] = 'a';
    src[1] = 'b';
    char* c = slide_copy(src, buf);
    *c = 0;  /* Unnecessary: Just to really hammer home the point. */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ab", src,
        "Source must remain unmodified.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ab", buf,
        "Buf should have identical contents to src.");
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(c, buf + strlen(src),
        "If characters written, return value should be the same as "
        "buf+strlen(src).");
}

void test_slide_copy_concatenate(void)
{
    buf[0] = 'a';
    buf[1] = 'b';
    src[0] = 'c';
    src[1] = 'd';
    char* c = slide_copy(src, buf + 2);
    *c = 0;  /* Unnecessary: Just to really hammer home the point. */
    TEST_ASSERT_EQUAL_STRING_MESSAGE("cd", src,
        "Source must remain unmodified.");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("abcd", buf,
        "Buf should be concatenated by src.");
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(c, buf + strlen(buf),
        "If characters written, return value should point to 'end' of buf.");
}

void test_slide_copy_termination(void)
{
    char* c = buf;
    char compBuf[BUF_SIZ];

    /* Remove the guardrails */
    size_t index;
    const char holder = 'X';
    for (index = 0; index < BUF_SIZ; index++)
    {
        buf[index] = holder;
        compBuf[index] = holder;
    }

    src[0] = 'a';
    src[1] = 'b';
    src[2] = 'c';

    c = slide_copy(src, c);
    c = slide_copy(src, c);

    /* (artifically) terminate both buffers at the end to make string
     * comparison easier - snprintf needs strlen for %s. */
    compBuf[BUF_SIZ - 1] = 0;
    buf[BUF_SIZ - 1] = 0;

    /* Create our comparator */
    snprintf(compBuf, BUF_SIZ, "%s%s%s", src, src, &compBuf[6]);

    TEST_ASSERT_EQUAL_STRING_MESSAGE(compBuf, buf,
        "Null terminator characters from src must not be copied to buf.");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_is_alphanum);
    RUN_TEST(test_slide_copy_empty);
    RUN_TEST(test_slide_copy_empty_buf);
    RUN_TEST(test_slide_copy_concatenate);
    RUN_TEST(test_slide_copy_termination);
    return UNITY_END();
}

#undef SRC_SIZ
#undef BUF_SIZ
