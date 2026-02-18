# History Module

## Purpose

Store and navigate command history. The history module is **owned by P1** and provides a clean API that the line editor (P4) consumes. History is a standalone data structure module — it has no dependency on terminal I/O, signals, or the executor.

## Data Structure

```c
typedef struct s_history_entry
{
    int             number;         // History number (1-based, monotonically increasing)
    char            *line;          // Command line string
    struct s_history_entry *prev;
    struct s_history_entry *next;
}   t_history_entry;

typedef struct s_history
{
    t_history_entry *head;          // Oldest entry
    t_history_entry *tail;          // Newest entry
    t_history_entry *current;       // Navigation cursor (NULL when not navigating)
    int             count;          // Number of entries
    int             max_size;       // Max entries to keep (default: 500)
    int             next_number;    // Next history number to assign
    char            *file_path;     // ~/.42sh_history
}   t_history;
```

## Interface

```c
// Lifecycle
void    history_init(t_history *hist, int max_size);
void    history_free(t_history *hist);

// Add a command to history (called after each accepted line)
int     history_add(t_history *hist, const char *line);

// Navigation (called by line editor on up/down arrow)
char    *history_prev(t_history *hist);
char    *history_next(t_history *hist);

// Reset navigation cursor to "not navigating" state
// Called before each new readline session
void    history_reset_cursor(t_history *hist);

// File persistence
int     history_load(t_history *hist, const char *path);
int     history_save(t_history *hist, const char *path);
```

## Operations

### history_add

Called by the line editor after the user presses Enter.

```
history_add(hist, line):
    # Skip empty or whitespace-only lines
    if line is empty or all whitespace:
        return 0

    # Deduplication: skip if identical to most recent entry
    if hist->tail is not NULL and hist->tail->line equals line:
        return 0

    # Create new entry
    entry = allocate t_history_entry
    entry->line = strdup(line)
    entry->number = hist->next_number++
    entry->prev = hist->tail
    entry->next = NULL

    # Link to list
    if hist->tail:
        hist->tail->next = entry
    else:
        hist->head = entry      # first entry
    hist->tail = entry
    hist->count++

    # Trim oldest if over max_size
    while hist->count > hist->max_size:
        old = hist->head
        hist->head = old->next
        if hist->head:
            hist->head->prev = NULL
        else:
            hist->tail = NULL   # list is now empty
        free old->line, free old
        hist->count--

    return 1
```

### history_prev

Navigate backward (older). Returns the line string, or NULL if already at oldest.

```
history_prev(hist):
    if hist->count == 0:
        return NULL

    if hist->current is NULL:
        # Start navigating from the newest entry
        hist->current = hist->tail
    else if hist->current->prev is not NULL:
        # Move to older entry
        hist->current = hist->current->prev
    else:
        # Already at oldest entry
        return NULL

    return hist->current->line
```

### history_next

Navigate forward (newer). Returns the line string, or NULL if past the newest (meaning "restore saved line").

```
history_next(hist):
    if hist->current is NULL:
        # Not navigating
        return NULL

    if hist->current->next is not NULL:
        # Move to newer entry
        hist->current = hist->current->next
        return hist->current->line
    else:
        # Past the newest entry — stop navigating
        hist->current = NULL
        return NULL             # caller restores saved_line
```

### history_reset_cursor

Reset navigation state. Called by the line editor before each new readline session (so a new up-arrow starts from the newest entry, not wherever the last session left off).

```
history_reset_cursor(hist):
    hist->current = NULL
```

### history_load

Load history from file at shell startup.

```
history_load(hist, path):
    expand ~ in path if needed
    fd = open(path, O_RDONLY)
    if fd < 0: return 0         # no history file is not an error

    for each line in the file:
        strip trailing newline
        if line is not empty:
            history_add(hist, line)

    close(fd)
    return 1
```

File format: one command per line, plain text. Most recent command is the last line.

### history_save

Save history to file at shell exit.

```
history_save(hist, path):
    expand ~ in path if needed
    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600)
    if fd < 0: return 0         # print warning, but don't crash

    entry = hist->head
    while entry:
        write entry->line to fd
        write '\n' to fd
        entry = entry->next

    close(fd)
    return 1
```

File permissions: `0600` (read/write owner only — history may contain sensitive commands).

### history_free

Free all entries. Called at shell exit after history_save.

```
history_free(hist):
    entry = hist->head
    while entry:
        next = entry->next
        free entry->line
        free entry
        entry = next
    hist->head = NULL
    hist->tail = NULL
    hist->current = NULL
    hist->count = 0
```

## Integration with Line Editor (P4)

The line editor uses history through a simple contract:

| When | Line editor calls | What happens |
|------|-------------------|--------------|
| Readline session starts | `history_reset_cursor(hist)` | Navigation cursor reset to NULL |
| User presses Up arrow | `history_prev(hist)` | Returns older line, or NULL at oldest |
| User presses Down arrow | `history_next(hist)` | Returns newer line, or NULL past newest |
| User presses Enter | `history_add(hist, line)` | Adds line to history (with dedup) |
| Shell startup | `history_load(hist, path)` | Loads history file |
| Shell exit | `history_save(hist, path)` | Saves history file |

The line editor is responsible for:
- Saving the current buffer before starting navigation (`saved_line`)
- Restoring `saved_line` when `history_next()` returns NULL
- Copying the returned line into the edit buffer

The history module is responsible for:
- Maintaining the doubly linked list
- Deduplication
- Max size enforcement
- File I/O

## Modular: History Expansions (P1)

If implementing the **History modular feature**, P1 adds a pre-processing step that runs on the raw input line BEFORE tokenization:

```
history_expand(hist, input):
    scan input for:
        !!          → replace with hist->tail->line
        !n          → replace with entry number n
        !-n         → replace with entry (count - n) from tail
        !string     → replace with most recent entry starting with string
        !?string    → replace with most recent entry containing string

    if any expansion happened:
        print the expanded line (so user sees what was substituted)

    return expanded string
```

This is called in the main loop after `line_editor_readline()` returns, before `lexer_tokenize()`.

## Files

```
include/history.h            # t_history, t_history_entry, all function declarations

src/history/
├── history.c                # history_init, history_free, history_add
├── history_nav.c            # history_prev, history_next, history_reset_cursor
├── history_file.c           # history_load, history_save
└── history_expand.c         # Modular: history expansion (!!, !n, etc.)
```
