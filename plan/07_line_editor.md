# Line Editor Module (Termcap)

## Purpose

Provide interactive line editing using termcap library. This replaces system `readline()` with a custom implementation that handles:

- Raw terminal mode
- Cursor movement
- Character insertion/deletion
- History navigation
- Multi-line input
- Signal handling during input

## Interface

```c
// Initialize line editor
int line_editor_init(t_line_editor *le, t_shell *shell);

// Cleanup
void line_editor_cleanup(t_line_editor *le);

// Read a line (main function) - returns malloc'd string or NULL for EOF
char *line_editor_readline(t_line_editor *le, const char *prompt);

// History API (provided by P1's history module — see include/history.h)
// The line editor USES these functions but does NOT implement them:
//   history_add(), history_prev(), history_next(),
//   history_reset_cursor(), history_load(), history_save()
```

## Terminal Modes

### Canonical Mode (Normal)
- Input is line-buffered (read returns after newline)
- Special characters processed by kernel (Ctrl-C sends SIGINT, etc.)
- Automatic echo of typed characters

### Raw Mode (For Line Editor)
- Character-by-character input
- No automatic echo
- We handle all special keys ourselves
- **Keep OPOST enabled** so `\n` still produces `\r\n` (bash does this too)

### Raw Mode Setup

```
enter_raw_mode(le):
    save current termios settings in le->orig_termios

    copy to raw settings, then modify:

    Input flags - clear:
        BRKINT    (no break signal)
        ICRNL     (don't map CR to NL - we handle Enter ourselves)
        INPCK     (no parity check)
        ISTRIP    (don't strip 8th bit)
        IXON      (no software flow control)

    Output flags:
        Keep OPOST ENABLED (so \n → \r\n works automatically)

    Control flags:
        Set CS8   (8 bits per byte)

    Local flags - clear:
        ECHO      (no auto echo)
        ICANON    (non-canonical mode, read byte-by-byte)
        IEXTEN    (no extended functions)
        ISIG      (no signal generation from Ctrl-C etc - we handle it)

    Control characters:
        VMIN = 1   (read returns after 1 byte)
        VTIME = 0  (no timeout)

    apply with tcsetattr(TCSAFLUSH)
    le->raw_mode = 1

exit_raw_mode(le):
    if le->raw_mode:
        restore le->orig_termios with tcsetattr(TCSAFLUSH)
        le->raw_mode = 0
```

## Termcap Initialization

Load terminal capabilities at startup:

```
Capabilities to load:
    cm  - cursor motion (tgoto)
    ce  - clear to end of line
    cd  - clear to end of screen
    cl  - clear screen
    up  - cursor up one line
    do  - cursor down one line
    le  - cursor left
    nd  - cursor right (non-destructive space)
    cr  - carriage return
    dc  - delete character
    co  - number of columns (tgetnum)
    li  - number of lines (tgetnum)

Init:
    term_name = getenv("TERM"), default to "xterm"
    tgetent(buf, term_name) to load terminal database
    tgetstr() for each string capability
    tgetnum() for numeric capabilities
    default co=80, li=24 if not found

Output: use tputs(capability, 1, putchar_fn)
    where putchar_fn writes one byte to STDOUT
```

## Key Reading

Read one keypress at a time. Map raw bytes to logical key values.

```
Key mapping:
    '\r' or '\n'  → KEY_ENTER
    127 or 8      → KEY_BACKSPACE
    '\t'          → KEY_TAB
    Ctrl-A (1)    → KEY_HOME (or KEY_CTRL_A)
    Ctrl-B (2)    → KEY_LEFT
    Ctrl-C (3)    → KEY_CTRL_C
    Ctrl-D (4)    → KEY_CTRL_D (EOF if empty)
    Ctrl-E (5)    → KEY_END
    Ctrl-F (6)    → KEY_RIGHT
    Ctrl-K (11)   → KEY_CTRL_K (kill to end)
    Ctrl-L (12)   → KEY_CTRL_L (clear screen)
    Ctrl-N (14)   → KEY_DOWN (next history)
    Ctrl-P (16)   → KEY_UP (prev history)
    Ctrl-U (21)   → KEY_CTRL_U (kill to start)
    Ctrl-W (23)   → KEY_CTRL_W (kill word back)
    27 (ESC)      → read escape sequence

Escape sequences (ESC [ ...):
    ESC [ A       → KEY_UP
    ESC [ B       → KEY_DOWN
    ESC [ C       → KEY_RIGHT
    ESC [ D       → KEY_LEFT
    ESC [ H       → KEY_HOME
    ESC [ F       → KEY_END
    ESC [ 1 ~     → KEY_HOME
    ESC [ 3 ~     → KEY_DELETE
    ESC [ 4 ~     → KEY_END
    32-126        → KEY_CHAR (printable character)
```

For escape sequences, read with a short timeout (or check if more bytes are available) to distinguish ESC key press from ESC sequence.

## Buffer Management

The line buffer is a dynamic char array with cursor position tracking:

```
Model:
    buffer[]  - the text content
    len       - current length of text
    cursor    - position of cursor (0 to len)
    buf_size  - allocated capacity

Operations:
    insert_char(c):
        grow buffer if needed (double capacity)
        shift chars from cursor..len right by 1
        put c at cursor position
        advance cursor, increment len

    delete_char_before():  (backspace)
        if cursor == 0: nothing
        shift chars from cursor..len left by 1
        decrement cursor and len

    delete_char_at():  (delete key)
        if cursor == len: nothing
        shift chars from cursor+1..len left by 1
        decrement len

    kill_to_end():
        set buffer[cursor] = '\0'
        len = cursor

    kill_to_start():
        shift chars from cursor..len to position 0
        len -= cursor
        cursor = 0

    clear():
        len = 0, cursor = 0, buffer[0] = '\0'
```

## Display Refresh

After each edit, redraw the line:

```
refresh_line(le):
    move cursor to start of input area (carriage return)
    write prompt
    write buffer contents
    clear to end of line (termcap 'ce')

    # Handle multi-line: if prompt + buffer > terminal width
    calculate actual cursor position:
        total_pos = prompt_len + cursor
        cursor_row = total_pos / terminal_cols
        cursor_col = total_pos % terminal_cols

    calculate current position (after writing buffer):
        current_pos = prompt_len + len
        current_row = current_pos / terminal_cols

    # Move cursor from end-of-buffer position back to cursor position
    move up (current_row - cursor_row) times using 'up' capability
    carriage return, then move right cursor_col times using 'nd' capability
```

## Main Read Loop

```
line_editor_readline(le, prompt):
    init buffer (empty)
    save history position
    enter raw mode (fallback to simple getline if fails)
    write prompt to stdout

    setup signal handlers for SIGINT (set flag, don't terminate)

    loop:
        key = read_key()

        KEY_CHAR:       insert_char, refresh
        KEY_ENTER:      write newline, break → return buffer
        KEY_CTRL_D:     if buffer empty → return NULL (EOF)
                        else → delete char at cursor
        KEY_CTRL_C:     write "^C\n", clear buffer, rewrite prompt
        KEY_BACKSPACE:  delete char before cursor, refresh
        KEY_DELETE:     delete char at cursor, refresh
        KEY_LEFT:       if cursor > 0: cursor--, refresh
        KEY_RIGHT:      if cursor < len: cursor++, refresh
        KEY_UP:         history_prev, refresh
        KEY_DOWN:       history_next, refresh
        KEY_HOME:       cursor = 0, refresh
        KEY_END:        cursor = len, refresh
        KEY_CTRL_U:     kill to start, refresh
        KEY_CTRL_K:     kill to end, refresh
        KEY_CTRL_W:     kill word backwards, refresh
        KEY_CTRL_L:     clear screen (termcap 'cl'), refresh

    exit raw mode
    return strdup(buffer)
```

## History Integration

The line editor uses P1's history module for up/down arrow navigation. See **`17_history.md`** for the full history module plan.

**How the line editor uses history:**
- On KEY_UP: call `history_prev(hist)`, copy returned line into buffer
- On KEY_DOWN: call `history_next(hist)`, copy returned line into buffer
- Before history navigation: save current buffer as `saved_line`
- When `history_next()` returns NULL (past newest): restore `saved_line`
- On KEY_ENTER (line accepted): call `history_add(hist, buffer)`
- On init: call `history_load(hist, "~/.42sh_history")`
- On exit: call `history_save(hist, "~/.42sh_history")`
- Before each navigation session: call `history_reset_cursor(hist)`

## Multi-line Input

When the user types an unclosed quote or ends with `\`, prompt for continuation:

```
line_editor_readline_continued(le, prompt):
    total = readline(prompt)

    while total has unclosed quotes or ends with '\':
        continuation = readline("> ")
        if continuation is NULL: return NULL (EOF)
        total = total + "\n" + continuation

    return total
```

Use `lexer_check_quotes()` to detect unclosed quotes.

## Files

```
src/line_editor/
├── line_editor.c        # Main readline function, init/cleanup
├── terminal.c           # Raw mode enter/exit, termcap init
├── keys.c               # Key reading and escape sequence parsing
├── buffer.c             # Buffer management (insert, delete, etc.)
└── display.c            # Screen refresh

# History lives in its own module (P1-owned):
src/history/
├── history.c            # History operations (add, prev, next, reset_cursor)
└── history_file.c       # File persistence (load/save)
```
