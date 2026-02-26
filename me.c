#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "keyb.h"
#include "state.h"
#include "term.h"

int headed_mode(int const argc, char const * const * const argv)
{
    /* Could do with an argument parser, but I'm lazy. */
    if (argc < 2)
    {
        fprintf(stderr, "Give me a read/write path!\n");
        return 1;
    }
    long lineopen = 0;
    if (argc > 2)
    {
        if (*argv[2] == '+')
        {
            /* Naive */
            char *endptr;
            lineopen = strtol(argv[2] + 1, &endptr, 10);
            if ((size_t)(endptr - argv[2] + 1) != strlen(argv[2] + 1) ||
                lineopen < 0)
                fprintf(stderr, "Line jump argument string '%s' could not be "
                        "interpreted. It should be in the form '+N' for "
                        "positive integer N.", argv[2]);
            lineopen = lineopen ? lineopen - 1 : 0;  /* one-based indexing */
        }
    }
    init_state(argv[1], (int)(lineopen > INT_MAX ? INT_MAX : lineopen));
    store_original_termios();
    term_setup();
    do redraw_screen(); while (proc_key(block_get_key()));
    clear_screen();
    restore_original_termios();
    destroy_state();
    return 0;
}

int main(int const argc, char const * const * const argv)
{return headed_mode(argc, argv);}
