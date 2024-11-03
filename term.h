#ifndef ME_TERM_H
#define ME_TERM_H
void clear_screen(void);
char* stage_draw_fci(char* const);
void redraw_screen(void);
void restore_original_termios(void);
void store_original_termios(void);
void term_setup(void);
void update_window_size(void);
#endif
