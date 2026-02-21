#ifndef ME_HELPERS_H
#define ME_HELPERS_H
void dump_chars_to_tmp_file(char const * const);
void dump_uint_to_tmp_file(unsigned const);
int is_alphanum(char const);
int is_space(char const);
char* slide_copy(char const *, char*);
#endif
