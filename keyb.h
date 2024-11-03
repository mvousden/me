#ifndef ME_KEYB_H
#define ME_KEYB_H
#define CTRL_(key) (key & 0x1F)  /* ASCII hackery */
unsigned block_get_key(void);
int proc_key(unsigned);
#endif
