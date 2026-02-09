# Signal Handling Module

## Purpose

Handle signals properly throughout the shell's lifecycle. Different signals require different handling depending on context (interactive prompt, executing command, etc.).

## Key Signals

| Signal | Default | Shell Behavior |
|--------|---------|----------------|
| SIGINT | Terminate | Interrupt current command, return to prompt |
| SIGQUIT | Core dump | Ignored in interactive shell |
| SIGTSTP | Stop | Stop foreground job |
| SIGTTIN | Stop | Ignored (shell handles terminal) |
| SIGTTOU | Stop | Ignored (shell handles terminal) |
| SIGCHLD | Ignored | Update job statuses |
| SIGTERM | Terminate | Exit shell gracefully |
| SIGHUP | Terminate | Send to jobs, then exit |
| SIGPIPE | Terminate | Ignored (handle EPIPE instead) |

## Always Use sigaction()

**Never use `signal()`.** Its behavior is non-portable (handler may reset to default after first signal on some systems). Always use `sigaction()`:

```c
// Helper pattern:
struct sigaction sa;
sa.sa_handler = handler_function;
sigemptyset(&sa.sa_mask);
sa.sa_flags = 0;
sigaction(SIGINT, &sa, NULL);  // optionally save old action
```

## Global Signal State

Use a volatile global for signal communication between handler and main code:

```c
extern volatile sig_atomic_t g_signal_received;
```

- `volatile`: prevents compiler from optimizing away reads
- `sig_atomic_t`: guaranteed atomic read/write

## Three Signal Contexts

### 1. Interactive Shell (at prompt)

```
signals_setup_interactive():
    SIGINT  → custom handler (set flag, write newline)
    SIGQUIT → SIG_IGN
    SIGTSTP → SIG_IGN
    SIGTTIN → SIG_IGN
    SIGTTOU → SIG_IGN
    SIGCHLD → custom handler (set flag for background job tracking)
    SIGTERM → custom handler (set flag to exit)
    SIGHUP  → custom handler (set flag to send HUP to jobs + exit)
```

SIGINT handler at prompt:
```
sigint_handler_interactive(sig):
    g_signal_received = SIGINT
    write(STDOUT, "\n", 1)      # only write() is safe in handler
```

### 2. During Command Execution (foreground)

```
signals_setup_executing():
    SIGINT  → SIG_IGN  (let foreground child receive it)
    SIGQUIT → SIG_IGN
    SIGTSTP → SIG_IGN  (let foreground child receive it)
    SIGCHLD → custom handler
```

The shell ignores these signals while a foreground command runs. The terminal sends SIGINT/SIGTSTP to the foreground process group (the job), not the shell.

### 3. In Child Process (before exec)

```
signals_setup_child():
    Restore ALL to SIG_DFL:
        SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU, SIGCHLD, SIGPIPE
```

Children must have default signal handling, because the shell ignores signals that children need to respond to.

## SIGCHLD Handler

```
sigchld_handler(sig):
    save errno (handler may change it)
    g_signal_received = SIGCHLD

    # Optionally: lightweight reaping with WNOHANG
    # But keep it simple - set flag and handle in main loop

    restore errno
```

## Checking Signals in Main Loop

Called before each prompt:

```
signals_check(shell):
    sig = g_signal_received
    g_signal_received = 0

    if sig == SIGINT:
        shell->last_exit_status = 130   # 128 + 2

    if sig == SIGCHLD:
        job_update_statuses(shell)
        job_notify(shell)

    if sig == SIGTERM:
        shell->running = false

    if sig == SIGHUP:
        for each running job: kill(-job.pgid, SIGHUP)
        shell->running = false
```

## Signal-Safe Functions

In signal handlers, ONLY use async-signal-safe functions:

**Safe:** `write()`, `_exit()`, `signal()`, `sigaction()`, `getpid()`, `kill()`, `waitpid()` (with WNOHANG)

**NOT Safe (never use in handlers):** `printf()`, `fprintf()`, `malloc()`, `free()`, `exit()`

## Integration with Line Editor

The line editor needs its own signal handling during raw mode:

```
During line_editor_readline:
    SIGINT → set g_signal_received = SIGINT
             (read() returns -1 with EINTR, or we check the flag after each read)

    On SIGINT: clear buffer, print "^C\n", redraw prompt

    SIGQUIT → SIG_IGN (ignore in interactive mode)

    On readline return: restore signals to interactive/executing context
```

Use `sigaction()` to save and restore previous handlers around the readline call.

## Handling Interrupts During Heredoc

```
During heredoc input:
    SIGINT → abort heredoc, free content, return NULL
    (Check g_signal_received after each line read)
```

## Terminal Reset on Exit

Always restore terminal state before exiting:

```
shell_cleanup_and_exit(shell, status):
    if interactive:
        restore terminal attributes (original_termios)
        give terminal back to shell's group

    send SIGHUP to all running background jobs
    save history
    free all resources
    exit(status)
```

## Interface

```c
void signals_init(t_shell *shell);
void signals_setup_interactive(void);
void signals_setup_executing(void);
void signals_setup_child(void);
void signals_check(t_shell *shell);
```

## Files

```
src/signals/
├── signals.c          # Init and context setup functions
├── signals_handlers.c # Handler implementations
└── signals_utils.c    # Helper functions
```
