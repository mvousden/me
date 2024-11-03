#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "term.h"

int triggered = 0;

void err(const char * const s)
{
    if (!triggered)
    {
        triggered = 1;
        perror(s);
        restore_original_termios();
    }
    exit(1);
}
