#ifndef ME_LINE_H
#define ME_LINE_H
#include <stddef.h>
#include <stdio.h>
struct Line
{
    char* content;  /* Even empty lines are not NULL. Array is
                     * null-terminated. */
    size_t bufSiz;
    size_t len;  /* Does not include null terminator */
    struct Line* prev;  /* NULL for line 1 */
    struct Line* next;  /* NULL for line at EOF */
};
/* Very few of these actually check for NULL arguments - you need to do that
 * yourself. */
void append_char(struct Line* const, const char);
void append_string(struct Line* const, const char* const);
void check_expand_line_buffer(struct Line* const, const size_t);
void delete_char_from_line(struct Line* const, const size_t);
void destroy_line(struct Line*);
void destroy_line_cascade(struct Line*);
void init_line(struct Line*, struct Line* const, struct Line* const);
void insert_char_into_line(struct Line* const, const char, const size_t);
void merge_line_with_next(struct Line* const);
void split_line(struct Line* const, const size_t, const char, unsigned);
#endif
