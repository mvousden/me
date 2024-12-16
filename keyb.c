#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "cmds.h"
#include "conf.h"
#include "error.h"
/* #include "helpers.h" */
#include "keyb.h"
#include "keyb_defs.h"
#include "line.h"
#include "state.h"

extern struct MeState state;
extern struct MeConf conf;

/* If we capture an escape while processing an escape sequence, replay it the
 * next time block_get_key is called. */
int escApp = 0;
unsigned block_get_key(void)
{
    union Key
    {
        unsigned out;
        char byte[4];
    } key = {0};
    size_t index;

    if (escApp)
    {
        escApp = 1;
        key.byte[0] = ESCAPE;
    }
    else
    {
        ssize_t bytes;
        do
        {
            /* Term setup: read times out each second/10. */
            bytes = read(STDIN_FILENO, &key.byte[0], 1);
            if (bytes == -1 && errno != EAGAIN) err("block_get_key/read");
        }
        while (bytes != 1);
    }

    /* If we got an escape, let's get more keypresses. */
    if (key.byte[0] == ESCAPE)
    {
        for (index = 1; index < 4; index++)
        {
            if (read(STDIN_FILENO, &key.byte[index], 1) != 1) break;
            else if (key.byte[index] == ESCAPE)
            {
                escApp = 1;
                key.byte[index] = 0;
                break;
            }
        }
    }

    return key.out;
}

int proc_key(const unsigned key)
{
    /* In case you care - you will need helpers.h. */
    /* dump_uint_to_tmp_file(key); */

    switch (key)
    {
    case ARR_LF:
    case CTRL_('b'):
        return cmd_move_chars_left(1, NULL);
    case ARR_RT:
    case CTRL_('f'):
        return cmd_move_chars_right(1, NULL);
    case ARR_UP:
    case CTRL_('p'):
        return cmd_move_lines_up(1);
    case ARR_DN:
    case CTRL_('n'):
        return cmd_move_lines_down(1);
    case CTRL_('q'):
        return cmd_quit();
    case NAV_HOME:
    case CTRL_('a'):
        return cmd_move_line_home();
    case NAV_END:
    case CTRL_('e'):
        return cmd_move_line_end();
    case CTRL_('m'):
        return cmd_split_line(' ', 0);
    case DEL:
    case CTRL_('d'):
        return cmd_delete_char(0);
    case BACKSPACE:
        return cmd_delete_char(-1);
    case CTRL_('s'):
        return cmd_save_file();
    case CTRL_ALT_('s'):
        return cmd_dump_state();
    case ALT_('<'):
        return cmd_move_doc_home();
    case ALT_('>'):
        return cmd_move_doc_end();
    case ALT_('f'):
        return cmd_move_word_right();
    case ALT_('b'):
        return cmd_move_word_left();
    case ALT_('\\'):
        return cmd_zap_whitespace();
    default:
        if (IS_PRINTABLE(key)) return cmd_insert_char(key);
        else return 1;
    }
}
