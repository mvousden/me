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
void append_char(struct Line* const, char const);
void append_string(struct Line* const, char const * const);
void check_expand_line_buffer(struct Line* const, size_t const);
void delete_char_from_line(struct Line* const, size_t const);
void destroy_line(struct Line*);
void destroy_line_cascade(struct Line*);
void init_line(struct Line*, struct Line* const, struct Line* const);
void insert_char_into_line(struct Line* const, char const, size_t const);
int lenint(struct Line const * const);
void merge_line_with_next(struct Line* const);
void split_line(struct Line* const, size_t const, char const, unsigned);
#endif
