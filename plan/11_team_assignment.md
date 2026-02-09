# Team Assignment

## Overview

Division of work for a **4-person team**. The key is clear module boundaries with well-defined interfaces so each person can develop and test independently.

## Team Structure

| Person | Primary Responsibility | Modules |
|--------|----------------------|---------|
| **P1** | Parsing & Grammar | Lexer, Parser, AST, Heredoc Collection |
| **P2** | Execution & Core | Executor (calls expander), Pipes, Redirections |
| **P3** | Variables & Expansion | Variables, Expander, Builtins (export, unset, set, cd, echo) |
| **P4** | Terminal & Jobs | Line Editor, Job Control, Signals, History, Builtins (jobs, fg, bg) |

### Builtin Ownership

Builtins are split by domain knowledge:

| Builtin | Owner | Reason |
|---------|-------|--------|
| `cd` | P3 | Modifies PWD/OLDPWD variables |
| `echo` | P3 | Simple, no dependencies |
| `exit` | P2 | Controls executor/main loop flow |
| `type` | P2 | Needs PATH search (shared with executor) |
| `export` | P3 | Core variable operations |
| `unset` | P3 | Core variable operations |
| `set` | P3 | Lists variables |
| `jobs` | P4 | Job control |
| `fg` | P4 | Job control |
| `bg` | P4 | Job control |

---

## Detailed Role Descriptions

### P1: Parsing Team

**Modules:** Lexer, Parser, AST, Heredoc Collection

**Responsibilities:**
- Define token types and implement tokenizer (quotes preserved in values)
- Define shell grammar and recursive descent parser
- Assignment detection (in parser, not lexer)
- Heredoc collection pass (walk AST after parsing, read heredoc content)
- Define AST node types and creation/destruction

**Dependencies:**
- None (first in pipeline)

**Delivers to:**
- Executor (AST with heredoc content filled in)

**Key Decisions:**
- Token structure (see 01_data_structures.md)
- AST node structure
- Grammar rules and operator precedence

**Files:**
```
include/lexer.h, include/parser.h, include/ast.h
src/lexer/*, src/parser/*
```

---

### P2: Execution Team

**Modules:** Executor, Redirections, Builtins (exit, type)

**Responsibilities:**
- Walk AST and execute commands
- Call expander (P3's code) for each command before execution
- Implement pipe handling (no double-fork)
- Implement redirections (>, >>, <, <<, >&, <&)
- Handle fork/exec and PATH search
- Handle && and || short-circuit logic
- Implement exit and type builtins

**Dependencies:**
- P1: AST structure
- P3: Expander functions (expand_command), Variables (var_get_environ for execve)
- P4: Job control (job_create, job_launch), Builtins (builtin_get for checking)

**Delivers to:**
- Main loop (exit status)
- P4 Job control (forked processes)

**Key Decisions:**
- Pipeline memory management
- When to fork vs run in-process (builtins)
- How to handle `VAR=value command` temporary assignments

**Files:**
```
include/executor.h
src/executor/*, src/builtins/builtin_exit.c, src/builtins/builtin_type.c
```

---

### P3: Expansion & Variables Team

**Modules:** Expander, Variables, Builtins (export, unset, set, cd, echo)

**Responsibilities:**
- Variable storage: linked list with get/set/unset/export
- Environment array building (lazy rebuild with env_dirty flag)
- Expander as a service: expand_word, expand_word_to_fields, expand_command
- Quote-aware expansion (walk raw string char-by-char)
- Field splitting on $IFS
- Builtins that modify variables: export, unset, set, cd, echo

**Dependencies:**
- P1: Token/AST structures (to know how quotes are stored in values)

**Delivers to:**
- P2: Expander functions, variable lookup, environment array
- All: var_get_value, var_set, etc.

**Key Decisions:**
- Variable storage structure (linked list vs hash table)
- How to track which characters were quoted during expansion
- IFS splitting rules

**Files:**
```
include/expander.h, include/variables.h
src/expander/*, src/variables/*
src/builtins/builtin_export.c, builtin_unset.c, builtin_set.c, builtin_cd.c, builtin_echo.c
```

---

### P4: Terminal & System Team

**Modules:** Line Editor, Job Control, Signals, History, Builtins (jobs, fg, bg)

**Responsibilities:**
- Raw terminal mode with termcap
- Key reading, buffer management, display refresh
- History navigation and file persistence
- Process group management and job tracking
- Signal setup for all three contexts (interactive, executing, child)
- Builtins: jobs, fg, bg

**Dependencies:**
- P3: Signals module needs to coordinate with line editor

**Delivers to:**
- Main loop (input line from line editor)
- P2: Job control functions (job_create, job_launch_foreground, etc.)

**Key Decisions:**
- Buffer management strategy
- History storage format
- Job notification timing
- Signal handler strategy (minimal handler, check in main loop)

**Files:**
```
include/line_editor.h, include/job_control.h, include/signals.h
src/line_editor/*, src/job_control/*, src/signals/*
src/builtins/builtin_jobs.c, builtin_fg.c, builtin_bg.c
```

---

## Interface Contracts

These are the key function signatures that cross module boundaries. All team members must agree on these before starting implementation.

### Lexer → Parser (P1 internal)

```c
t_token *lexer_tokenize(const char *input);
void    token_list_free(t_token *head);
```

### Parser → Executor (P1 → P2)

```c
t_ast   *parser_parse(t_token *tokens);
void    ast_free(t_ast *node);
int     parser_collect_heredocs(t_ast *ast, t_shell *shell);
```

### Expander → Executor (P3 → P2)

```c
int     expand_command(t_shell *shell, t_cmd *cmd);
char    *expand_word(t_shell *shell, const char *word);
char    **expand_word_to_fields(t_shell *shell, const char *word);
```

### Variables → Everyone (P3 → All)

```c
char    *var_get_value(t_shell *shell, const char *name);
int     var_set(t_shell *shell, const char *name, const char *value);
int     var_unset(t_shell *shell, const char *name);
int     var_export(t_shell *shell, const char *name);
char    **var_get_environ(t_shell *shell);
```

### Line Editor → Main (P4 → Main)

```c
char    *line_editor_readline(t_line_editor *le, const char *prompt);
```

### Builtins → Executor (P3/P4 → P2)

```c
t_builtin_fn builtin_get(const char *name);
int     builtin_is_builtin(const char *name);
```

### Job Control → Executor (P4 → P2)

```c
t_job   *job_create(t_shell *shell, const char *cmd_line);
int     job_launch_foreground(t_shell *shell, t_job *job);
int     job_launch_background(t_shell *shell, t_job *job);
void    job_add_process(t_job *job, pid_t pid);
```

---

## Communication Plan

### Before Coding
- Agree on all data structures (01_data_structures.md)
- Agree on interface contracts above
- Set up shared header files with function prototypes and struct definitions

### During Development
- Each person works on their own feature branch
- Merge to main when a module is testable
- Integration testing happens on main branch

### Git Workflow

```
main
├── feature/lexer-parser      (P1)
├── feature/executor          (P2)
├── feature/expander-vars     (P3)
├── feature/line-editor       (P4)
├── feature/job-control       (P4)
└── feature/signals           (P4)
```

---

## Milestones

### M1: Basic Pipeline (P1 + P2)
- Lexer tokenizes simple commands
- Parser builds AST for simple commands
- Executor runs simple commands via fork/exec
- Test: `ls`, `ls -la`, `/bin/echo hello`

### M2: Pipes and Redirections (P1 + P2)
- Lexer/Parser handle |, >, >>, <
- Executor implements pipes and redirections
- Test: `ls | grep txt`, `echo hello > file`

### M3: Variables and Expansion (P3)
- Variable storage working
- Basic $VAR expansion working
- export/unset/set builtins working
- Test: `VAR=hello`, `echo $VAR`, `export VAR`

### M4: Line Editing (P4)
- Raw mode, key reading, buffer management
- Arrow keys, backspace, history
- Test: interactive editing works

### M5: Integration (All)
- Executor calls expander before running commands
- Builtins integrated (cd, echo, exit, type)
- Test: `echo $HOME`, `cd /tmp && pwd`

### M6: Job Control (P4 + P2)
- Background jobs, fg/bg/jobs builtins
- Signal handling in all contexts
- Test: `sleep 10 &`, `jobs`, `fg %1`, Ctrl-C, Ctrl-Z

### M7: Complete Mandatory (All)
- Heredoc, fd duplication, logical operators
- All edge cases handled
- Memory leak and crash testing

### M8+: Modular Features (All)
- Pick and implement 6 features
- Each person takes features related to their domain
