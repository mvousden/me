/* Assumes LE... */
#define ALT_(key) (key << 8 | 0x1b)
#define ARR_UP 4283163       /* \x1b[A, or 0d27,0d91,0d65,0d0 */
#define ARR_DN 4348699       /* \x1b[B, or 0d27,0d91,0d66,0d0 */
#define ARR_LF 4479771       /* \x1b[C, or 0d27,0d91,0d68,0d0 */
#define ARR_RT 4414235       /* \x1b[D, or 0d27,0d91,0d67,0d0 */
#define BACKSPACE 127
#define CTRL_(key) (key & 0x1F)
#define CTRL_ALT_(key) ((key & 0x1f) << 8 | 0x1b)  /* Obviously */
#define DEL 2117294875       /* \x1b[3~, or 0d27,0d91,0d51,0d126 */
#define ESCAPE 27            /* \x1b, or 0d27 */
#define IS_PRINTABLE(key) (key >> 5 && !(key & 0xffffff80))  /* 0x20 to 0x7f */
#define NAV_HOME 2117163803  /* \x1b[1~, or 0d27,0d91,0d49,0d126 */
#define NAV_END 2117360411   /* \x1b[4~, or 0d27,0d91,0d52,0d126 */
