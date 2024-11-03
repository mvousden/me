Mark's Editor
===

An editor written by Mark, for Mark, so he can customise it as he needs to. It
lacks a bunch of features one would expect from a text editor, but that's fine,
because Mark can simply implement them when he wants/needs them.

Why would you do this?
===

A reasonable question. As the emacs-vi/m/neovim war rages on, a bunch of other
terminal text editors have been created - Wikipedia's listing is enormous, and
yet not comprehensive. Surely I could just use one of those? Of course I
could. I've been using emacs and vi interchangably for the last fifteen years
or so. I went through a phase using the wonderful mle text editor (adsr/mle on
GH for the curious) with many of the wonderful features it introduces. There is
no feature that I want to add to this editor that cannot be found elsewhere.

Fundamentally, this is not something I need to do, rather something I want to
do (and I wanted to learn more about how TUI software can be developed without
libraries like curses/ncurses).

And isn't that reason enough?

What's the game, then?
---

 - I care very little about compatibility.
 - I care a lot about stability.
 - I care about being able to fix it if I break it.
 - I care about editing ASCII text - this is not an IDE.

What me can do
===

 - Work in a terminal
 - Save written text to a fixed file
 - Be fairly conservative about memory, without writing temporary files
 - Support elementary keyboard shortcuts
 - Be unit-tested, somewhat
 - Word/line/document-jumping
 - Write buffer content dump (and some state) to binary file
 - Opening existing files

What it can't do, that Mark would like it to do
===

 - Handle files with more lines than fit in the terminal window with some
   scrolling mechanism.
 - Handle long lines sensibly, either by wrapping or truncating
 - Whitespace-aware newline by e.g. following the indentation of the previous
   line.
 - Toggle-able binary editing mode
 - Undo logic (via command pattern most likely)
 - Goto line shortcut
 - Whitespace-clearing shortcuts
 - Word-wrap mechanism (e.g. M-q (emacs))
 - Save written text to a particular path
 - Unit tests for saving and opening
 - Dumb search and replace
 - Regex search and replace (using a convenient regex library)
 - A tab key that doesn't suck, (to be clear, I don't want autocompletion), and
   sensible support for tab characters depending on editing mode down the line.
 - Treat certain files differently, i.e. editing modes (particularly for C!)
 - Syntax highlighting (nice-to-have)

What it can't do, and probably will never do
===

 - External scripting language support (with the possible exception of a
   configurator)
 - Non-ASCII characters

Keyboard shortcuts, in emacs-ese
===

 - C-b/C-f: move cursor one char left/right
 - C-p/C-n: move cursor one line up/down
 - C-q: quit the editor without confirmation
 - C-a: move cursor to start of line
 - C-e: move cursor to end of line
 - C-m: return synonym
 - C-d: delete one character ahead
 - C-s: save to file
 - C-M-s: state dump to fixed path
 - M-</M->: move cursor to start/end of document
 - M-f/M-b: move cursor to end/start of next/previous word

Not bugs
===

 - *Hanging cursor*: When moving the cursor from the end of one long line to
   one short line, the cursor remains in the same column. The cursor is then
   "hanging" over the end of the line. This is fine - any visible character
   inserted at that point will be appended to the selected line, and the cursor
   will snap back to the end of the line.
