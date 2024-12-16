#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"

/* Dump null-terminated array of characters to a file named 'tmp'. Clobbers, no
 * checking - mostly a debugging utility for terminal characters. */
void dump_chars_to_tmp_file(const char* const buf)
{
    FILE* tmp = fopen("tmp", "w");
    fwrite(buf, sizeof(char), strlen(buf), tmp);
    fclose(tmp);
}

/* Dump uint and its constituent bytes (4-byte unsigneds only) to a file named
 * 'tmp'. Clobbers, no checking - mostly a debugging utility for terminal
 * characters. */
void dump_uint_to_tmp_file(const unsigned key)
{
    union Key
    {
        unsigned u;
        char c[4];
    } un;
    char str[5];
    size_t ix;
    FILE* tmp;
    un.u = key;
    for (ix = 0; ix < 4; ix++) str[ix] = un.c[ix];
    str[4] = 0;
    tmp = fopen("tmp", "w");
    fprintf(tmp,
            "unsigned value: %010u, %08x\n"
            "bytes (dec): 0d%03u, 0d%03u, 0d%03u, 0d%03u\n"
            "bytes (hex): 0x%02x,  0x%02x,  0x%02x,  0x%02x\n"
            "str: \"%s\"\n",
            un.u, un.u,
            un.c[0], un.c[1], un.c[2], un.c[3],
            un.c[0], un.c[1], un.c[2], un.c[3],
            str);
    fclose(tmp);
}

/* ASCII */
int is_alphanum(const char c)
{
    return ((c > 0x2f && c < 0x3a) ||  /* digits */
            (c > 0x40 && c < 0x5b) ||  /* upper-case */
            (c > 0x60 && c < 0x7b));   /* lower-case */
}

int is_space(const char c){return c == 0x20;}

/* Dirty string copying function that copies NULL-terminated src to buf, then
 * returns a pointer one character off the end of buf. Does no checking of any
 * kind, and does not NULL-terminate the buffer (really). */
char* slide_copy(const char* src, char* buf)
{
    while (*src) *buf++ = *src++;
    return buf;
}
