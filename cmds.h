#ifndef ME_CMDS_H
#define ME_CMDS_H
int cmd_delete_char(const int);
int cmd_dump_state(void);
int cmd_insert_char(const unsigned);
int cmd_move_chars_left(unsigned, unsigned* const);
int cmd_move_chars_right(unsigned, unsigned* const);
int cmd_move_doc_end(void);
int cmd_move_doc_home(void);
int cmd_move_line_end(void);
int cmd_move_line_home(void);
int cmd_move_lines_up(unsigned);
int cmd_move_lines_down(unsigned);
int cmd_move_word(const unsigned);
int cmd_move_word_left(void);
int cmd_move_word_right(void);
int cmd_save_file(void);
int cmd_split_line(const char, const unsigned);
int cmd_quit(void);
int cmd_zap_whitespace(void);

int move_char(const unsigned, unsigned* const);
#endif
