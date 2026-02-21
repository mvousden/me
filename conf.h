#ifndef ME_CONF_H
#define ME_CONF_H
#include <stddef.h>
struct MeConf
{
    /* Lines and buffers */
    size_t const initLineSize;  /* Greater than zero. */
    size_t const vt100BufSize;  /* Greater than zero, arbitrary. */
    size_t const openChunkSize;  /* Greater than zero - fread chunk size. */

    /* Terminal properties */
    int const colOffset;
    int const lineOffset;

    /* Fill column indicator properties */
    char const * fciStrPre;
    char const  fciChar;
    char const * fciStrPost;
    int fciCol;
};
#endif
