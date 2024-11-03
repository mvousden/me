#ifndef ME_CONF_H
#define ME_CONF_H
#include <stddef.h>
struct MeConf
{
    /* Lines and buffers */
    const size_t initLineSize;  /* Greater than zero. */
    const size_t vt100BufSize;  /* Greater than zero, arbitrary. */
    const size_t openChunkSize;  /* Greater than zero - fread chunk size. */

    /* Terminal properties */
    const short colOffset;
    const short lineOffset;

    /* Fill column indicator properties */
    const char* fciStrPre;
    const char fciChar;
    const char* fciStrPost;
    unsigned short fciCol;
};
#endif
