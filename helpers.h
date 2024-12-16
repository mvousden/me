#ifndef ME_HELPERS_H
#define ME_HELPERS_H
void dump_chars_to_tmp_file(const char* const);
void dump_uint_to_tmp_file(const unsigned);
int is_alphanum(const char);
int is_space(const char);
char* slide_copy(const char*, char*);
#endif
