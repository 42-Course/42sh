# 42sh Architecture Overview

## Philosophy

1. **Stability over features** - A shell that never crashes is better than one with many features that segfaults
2. **Modular design** - Each component should be independent and testable
3. **Clear interfaces** - Well-defined boundaries between modules enable parallel development
4. **Reference: bash** - When in doubt, follow bash behavior

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                            42sh Main Loop                           │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                          LINE EDITOR                                │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                 │
│  │   Termcap   │  │   History   │  │  Input Buf  │                 │
│  └─────────────┘  └─────────────┘  └─────────────┘                 │
└─────────────────────────────────────────────────────────────────────┘
                                    │ char *line
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                             LEXER                                   │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                 │
│  │ State Mach. │  │  Tokenizer  │  │ Quote Track │                 │
│  └─────────────┘  └─────────────┘  └─────────────┘                 │
└─────────────────────────────────────────────────────────────────────┘
                                    │ t_token *tokens
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                             PARSER                                  │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐                 │
│  │  Grammar    │  │ AST Builder │  │Syntax Check │                 │
│  └─────────────┘  └─────────────┘  └─────────────┘                 │
└─────────────────────────────────────────────────────────────────────┘
                                    │ t_ast *ast
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                       HEREDOC COLLECTION                            │
│  Walk AST, read heredoc content from input for each << redirect     │
└─────────────────────────────────────────────────────────────────────┘
                                    │ t_ast *ast (with heredoc content)
                                    ▼
┌─────────────────────────────────────────────────────────────────────┐
│                            EXECUTOR                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌───────────┐  │
│  │  Commands   │  │   Pipes     │  │ Redirects   │  │  Builtins │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  └───────────┘  │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐    │
│  │  EXPANDER (called per-command, not as a separate pass)      │    │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌───────────────┐  │    │
│  │  │ Var Exp. │ │ Tilde    │ │ Globbing │ │ Quote Removal │  │    │
│  │  └──────────┘ └──────────┘ └──────────┘ └───────────────┘  │    │
│  └─────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────┘
                                    │
                    ┌───────────────┼───────────────┐
                    ▼               ▼               ▼
            ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
            │ Job Control │ │  Variables  │ │   Signals   │
            └─────────────┘ └─────────────┘ └─────────────┘
```

**Key design decision:** The expander is NOT a separate pipeline pass. It is a service called by the executor for each command's words right before that command runs. This is required because `$?` and other variables must reflect the state at execution time, not parse time.

## Data Flow

1. **Input**: Line editor reads user input character by character
2. **Lexing**: Raw string is converted to a linked list of tokens (quotes preserved in values)
3. **Parsing**: Tokens are organized into an Abstract Syntax Tree (AST)
4. **Heredoc collection**: Walk AST, read heredoc content for each `<<` redirect
5. **Execution**: AST is walked; each command's words are expanded immediately before that command runs
6. **Result**: Exit status is captured, shell state is updated

## Module Responsibilities

| Module | Input | Output | Responsibility |
|--------|-------|--------|----------------|
| Line Editor | keystrokes | `char *line` | Interactive input with editing |
| Lexer | `char *line` | `t_token *list` | Tokenization (preserves quotes in values) |
| Parser | `t_token *list` | `t_ast *tree` | Build syntax tree, detect assignments |
| Expander | `char *word` | `char **fields` | Variable/tilde/glob expansion (called by executor) |
| Executor | `t_ast *tree` | `int exit_status` | Walk AST, expand per-command, run commands |
| Builtins | `char **argv` | `int exit_status` | Built-in commands |
| Variables | get/set requests | values | State management |
| Job Control | process events | job status | Background jobs |
| Signals | OS signals | actions | Signal handling |

## Modes of Operation

The shell operates in two modes:

1. **Interactive mode** (`isatty(stdin)`): Show prompt, use line editor, handle signals for user
2. **Non-interactive mode** (`42sh -c "cmd"` or piped input): Read from string/stdin, no prompt, no line editor. Essential for testing.

## Shared State: t_shell

All modules access a shared shell state structure:

```c
typedef struct s_shell
{
    // Environment
    char            **env;              // Cached environment array (for execve)
    int             env_dirty;          // Needs rebuild?
    t_var           *variables;         // Internal shell variables

    // State
    int             last_exit_status;   // $?
    int             interactive;        // Is TTY?
    int             running;            // Main loop flag
    int             exit_confirmed;     // For double-exit with stopped jobs

    // Job Control
    t_job           *jobs;              // Job list
    t_job           *current_job;       // Most recent job (%+)
    pid_t           shell_pgid;         // Shell's process group
    int             terminal_fd;        // Terminal file descriptor
    struct termios  original_termios;   // Original terminal settings

    // Line Editor
    t_history       *history;           // Command history

    // Aliases (if implemented)
    t_alias         *aliases;           // Alias table

    // Hash table (if implemented)
    t_hash          *cmd_hash;          // Command path cache
}   t_shell;
```

## File Organization

```
42sh/
├── Makefile
├── include/
│   ├── shell.h           # Main header, t_shell
│   ├── lexer.h           # Lexer types and functions
│   ├── parser.h          # Parser types and functions
│   ├── ast.h             # AST node definitions
│   ├── expander.h        # Expander functions
│   ├── executor.h        # Executor functions
│   ├── builtins.h        # Builtin declarations
│   ├── variables.h       # Variable management
│   ├── job_control.h     # Job control
│   ├── signals.h         # Signal handling
│   ├── line_editor.h     # Line editing
│   └── utils.h           # Utility functions
├── src/
│   ├── main.c            # Entry point, main loop
│   ├── lexer/
│   ├── parser/
│   ├── expander/
│   ├── executor/
│   ├── builtins/
│   ├── variables/
│   ├── history/          # History module (P1-owned, used by line editor)
│   ├── job_control/
│   ├── signals/
│   ├── line_editor/
│   └── utils/
├── libft/                # Your libft library
├── tests/                # Test scripts
└── plan/                 # This documentation
```

## Error Handling Strategy

1. **Lexer errors**: Invalid tokens → print error, return NULL
2. **Parser errors**: Syntax errors → print error, return NULL
3. **Expansion errors**: Bad substitution → print error, set $? to 1
4. **Execution errors**: Command not found, permission denied → print error, set $?
5. **Memory errors**: malloc fails → clean up and exit gracefully

Never crash. Always free memory. Always reset terminal state.
