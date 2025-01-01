#include <stdio.h>

#include "keyb.h"
#include "state.h"
#include "term.h"

int headed_mode(const int argc, const char* const * const argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Give me a read/write path!\n");
        return 1;
    }
    init_state(argv[1]);
    store_original_termios();
    term_setup();
    do redraw_screen(); while (proc_key(block_get_key()));
    clear_screen();
    restore_original_termios();
    destroy_state();
    return 0;
}

int main(const int argc, const char* const * const argv)
{return headed_mode(argc, argv);}
