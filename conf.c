#include "conf.h"
const struct MeConf conf = {
    /* initLineSize */  256,
    /* vt100BufSize */  5000,
    /* openChunkSize */ 1024,  /* 1 KiB */
    /* colOffset */     1,
    /* lineOffset */    1,
    /* fciStrPre */     "\x1b[38;2;100;0;0m",  /* Dark red */
    /* fciChar */       '|',
    /* fciStrPost */    "\x1b[38;2;146;131;116m",  /* Default */
    /* fciCol */        80
};
