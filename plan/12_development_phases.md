# Development Phases

## Overview

This document outlines the recommended development order. The key principle is: **always have something working**. Each phase builds on the previous one and produces a testable result.

---

## Phase 0: Setup & Foundations

### Goals
- Project structure
- Build system
- Shared data structures
- Basic utilities

### Tasks
1. Create directory structure (see 00_overview.md for layout)
2. Setup Makefile (with standard rules: all, clean, fclean, re)
3. Define shared header files with all struct definitions
4. Implement basic utilities (string helpers, memory helpers)
5. Define `t_shell` structure
6. Create main loop skeleton
7. **Non-interactive mode**: Accept `-c "command"` flag for testing

### Deliverable

```
Main loop (pseudo code):

shell_init(shell, envp)

if "-c" flag given:
    shell->interactive = false
    process the command string directly
    exit

shell->interactive = isatty(STDIN)

while shell->running:
    if interactive:
        line = line_editor_readline(prompt)    # use system readline temporarily
    else:
        line = read_line_from_stdin()

    if line is NULL: break (EOF)
    if line is empty: continue

    tokens = lexer_tokenize(line)
    if tokens is NULL: continue (error already printed)

    ast = parser_parse(tokens)
    free tokens
    if ast is NULL: continue

    parser_collect_heredocs(ast, shell)

    executor_execute(shell, ast)
    free ast

shell_cleanup(shell)
exit(shell->last_exit_status)
```

### Checklist
- [ ] Directory structure created
- [ ] Makefile compiles project
- [ ] libft integrated
- [ ] Shell starts and shows prompt
- [ ] `-c "echo hello"` works (non-interactive mode)
- [ ] Ctrl-D exits

---

## Phase 1: Minimal Execution

### Goals
- Execute simple commands
- Basic PATH search
- Basic error handling
- Non-interactive `-c` mode testable

### Tasks
1. Simple lexer (split on whitespace, recognize TOK_WORD)
2. Simple parser (build NODE_COMMAND with argv)
3. Simple executor (fork + exec)
4. PATH search (split PATH on ':', try each directory)
5. Error messages (command not found, permission denied)

### Deliverable
```
42sh -c "/bin/ls"          # works
42sh -c "ls"               # works (PATH search)
42sh -c "ls -la"           # works (arguments)
42sh -c "nonexistent"      # 42sh: nonexistent: command not found
```

### Checklist
- [ ] Can execute commands with absolute path
- [ ] PATH search works
- [ ] Arguments passed correctly
- [ ] "command not found" error
- [ ] Non-interactive mode works for testing

---

## Phase 2: Redirections

### Goals
- Input/output redirections
- Append redirect
- Error handling for file operations

### Tasks
1. Lexer: recognize >, >>, <
2. Parser: parse redirections, attach to command node
3. Executor: apply redirections with dup2 before exec
4. Error handling (file not found, permission denied)

### Deliverable
```
42sh> echo hello > file.txt
42sh> cat file.txt
hello
42sh> echo world >> file.txt
42sh> cat < file.txt
hello
world
42sh> cat < nonexistent
42sh: nonexistent: No such file or directory
```

### Checklist
- [ ] Output redirection (>)
- [ ] Append redirection (>>)
- [ ] Input redirection (<)
- [ ] Error handling

---

## Phase 3: Pipes

### Goals
- Single pipe working
- Multiple pipes working
- Pipeline exit status (from last command)

### Tasks
1. Lexer: recognize |
2. Parser: build NODE_PIPE tree
3. Executor: create pipes, fork children, connect fds (no double-fork)
4. Wait for all children, return last command's status

### Deliverable
```
42sh> ls | grep txt
42sh> cat file.txt | head -5 | wc -l
42sh> ls | nonexistent
42sh: nonexistent: command not found
```

### Checklist
- [ ] Single pipe works
- [ ] Multiple pipes work
- [ ] Exit status is from last command
- [ ] Error in pipeline handled

---

## Phase 4: Basic Builtins

### Goals
- cd, echo, exit working
- Builtin detection (run in-process, not forked)

### Tasks
1. Builtin registry (name → function pointer table)
2. Builtin detection in executor: check before fork
3. Implement cd (with HOME, OLDPWD, PWD)
4. Implement echo (with -n flag)
5. Implement exit (with numeric argument, "too many arguments")

### Deliverable
```
42sh> cd /tmp
42sh> echo $PWD         # needs Phase 5, but cd itself works
42sh> cd
42sh> echo hello world
hello world
42sh> echo -n no newline
no newline42sh> exit
```

### Checklist
- [ ] cd works (with -, OLDPWD)
- [ ] echo works (with -n)
- [ ] exit works (with status)
- [ ] Builtins run in-process (not forked) when not in pipeline

---

## Phase 5: Variables & Expansion

### Goals
- Internal variables
- Environment variables
- Variable expansion ($VAR, ${VAR}, $?)
- Expander integration into executor

### Tasks
1. Variable storage (linked list: name, value, exported flag)
2. var_set, var_get_value, var_unset, var_export
3. var_init_from_environ (initialize from envp, set SHLVL, PWD)
4. var_get_environ (build char** for execve, lazy rebuild)
5. export, unset, set builtins
6. Expander: expand_word (handle $VAR, ${VAR}, $?)
7. Expander: expand_command (called by executor before each command)
8. Quote-aware expansion (single quotes = literal, double quotes = expand $)

### Deliverable
```
42sh> VAR=hello
42sh> echo $VAR
hello
42sh> export VAR
42sh> env | grep VAR
VAR=hello
42sh> unset VAR
42sh> echo $VAR

42sh> false
42sh> echo $?
1
```

### Checklist
- [ ] Internal variables work
- [ ] export works
- [ ] unset works
- [ ] set lists variables
- [ ] $VAR expansion works
- [ ] ${VAR} expansion works
- [ ] $? works
- [ ] Expander is called by executor per-command

---

## Phase 6: Logical Operators & Sequences

### Goals
- && operator (short-circuit AND)
- || operator (short-circuit OR)
- ; operator (sequence)
- Proper precedence: `;` < `&&`/`||` < `|`

### Tasks
1. Lexer: recognize &&, ||, ;
2. Parser: handle precedence in recursive descent
3. Executor: implement short-circuit logic for AND/OR nodes

### Deliverable
```
42sh> true && echo yes
yes
42sh> false && echo yes
42sh> false || echo no
no
42sh> echo one; echo two
one
two
42sh> false && echo a || echo b
b
```

### Checklist
- [ ] && works correctly
- [ ] || works correctly
- [ ] ; works correctly
- [ ] Precedence is correct

---

## Phase 7: Advanced Redirections & Heredoc

### Goals
- Heredoc (<<)
- File descriptor duplication (>&, <&)
- fd numbers (2>, 2>&1)
- Heredoc collection pass

### Tasks
1. Lexer: recognize <<, >&, <&, io_number before redirect
2. Parser: store io_number on redirect nodes, recognize heredoc delimiter
3. **Heredoc collection**: After parsing, walk AST and read heredoc content
   - For each `<<` redirect, prompt for input until delimiter
   - Store content in redir->heredoc_content
   - Handle SIGINT during heredoc input (abort)
4. Executor: write heredoc content to pipe, apply fd duplication
5. Handle `>&-` and `<&-` for closing fds

### Deliverable
```
42sh> cat << EOF
> hello
> world
> EOF
hello
world
42sh> ls nonexistent 2>&1 | cat
ls: nonexistent: No such file or directory
42sh> ls . nonexistent > out.txt 2>&1
```

### Checklist
- [ ] Heredoc works (with expansion in content when delimiter unquoted)
- [ ] Heredoc collection happens after full parse, before execution
- [ ] >& works
- [ ] <& works
- [ ] fd numbers work (2>, 2>&1)
- [ ] SIGINT during heredoc aborts cleanly

---

## Phase 8: Line Editor (Termcap)

### Goals
- Replace system readline with custom termcap implementation
- Arrow keys for cursor movement
- Arrow keys for history navigation

### Tasks
1. Termcap initialization (load terminal capabilities)
2. Raw terminal mode (enter/exit)
3. Key reading (single chars, escape sequences)
4. Buffer management (insert, delete, cursor tracking)
5. Display refresh (using termcap capabilities)
6. History integration (up/down arrows, add to history)
7. Ctrl-C (clear line), Ctrl-D (EOF), Ctrl-L (clear screen)

### Deliverable
- Left/right arrows move cursor within line
- Up/down arrows navigate history
- Backspace deletes character before cursor
- Home/End move to start/end of line
- Ctrl-C clears current line and shows new prompt

### Checklist
- [ ] Raw mode enter/exit works
- [ ] Arrow keys work
- [ ] History navigation works
- [ ] Ctrl-C, Ctrl-D, Ctrl-L work
- [ ] Multi-line display works (wrapping)
- [ ] Terminal state always restored on exit

---

## Phase 9: Job Control

### Goals
- Background jobs (&)
- jobs/fg/bg builtins
- Ctrl-Z support (SIGTSTP)
- Process groups

### Tasks
1. Shell takes its own process group and terminal (job_control_init)
2. Each job runs in its own process group (setpgid in child)
3. Foreground jobs get the terminal (tcsetpgrp)
4. Background job tracking and notification
5. jobs builtin (list jobs with status)
6. fg builtin (continue in foreground, give terminal)
7. bg builtin (continue in background, send SIGCONT)
8. SIGTSTP handling (stop foreground job)
9. SIGCHLD handling (update background job status)

### Deliverable
```
42sh> sleep 100 &
[1] 12345
42sh> jobs
[1]+ Running    sleep 100 &
42sh> fg %1
sleep 100
^Z
[1]+ Stopped    sleep 100
42sh> bg %1
[1]+ sleep 100 &
```

### Checklist
- [ ] Background execution works
- [ ] jobs lists jobs
- [ ] fg brings to foreground
- [ ] bg continues in background
- [ ] Ctrl-Z stops foreground job

---

## Phase 10: Signal Handling & Final Mandatory

### Goals
- Complete signal handling for all contexts
- type builtin
- Polish and testing

### Tasks
1. Signal setup: interactive context (at prompt)
2. Signal setup: executing context (foreground command running)
3. Signal setup: child context (restore defaults before exec)
4. Always use sigaction(), never signal()
5. Implement type builtin
6. Memory leak testing (valgrind)
7. Integration testing against bash
8. Edge case handling

### Deliverable
```
42sh> type echo
echo is a shell builtin
42sh> type ls
ls is /bin/ls
42sh> type nonexistent
type: nonexistent: not found
42sh> # Ctrl-C at prompt shows new line, doesn't exit
42sh> # Ctrl-C during command kills command, not shell
42sh> # Ctrl-\ ignored at prompt
```

### Checklist
- [ ] type works correctly
- [ ] Signals correct in all three contexts
- [ ] No memory leaks (valgrind clean)
- [ ] No crashes (segfault, bus error, double free)
- [ ] All mandatory features complete
- [ ] Terminal always restored on exit

---

## Phase 11+: Modular Features

Implement **6 features** from the modular list. See `13_modular_features.md` for details on each.

Suggested order based on dependencies:
1. **Inhibitors** first (affects expansion of everything else)
2. **Tilde expansion** (easy, high value)
3. **Aliases** (easy, self-contained)
4. **Globbing** (self-contained in expander)
5. **History expansions** (builds on existing history)
6. **test builtin** or **hash table** (self-contained)

---

## Testing Strategy

### Comparison Tests (Primary)

The most effective test: run the same command in bash and 42sh, compare output and exit status.

```
bash -c "command here; echo EXIT:\$?"
./42sh -c "command here; echo EXIT:\$?"
```

This is why non-interactive mode (`-c` flag) is implemented in Phase 0.

### Unit Tests
- Test each module independently
- Lexer: verify token sequences for various inputs
- Parser: verify AST structure
- Expander: verify expansion results
- Variables: verify get/set/export behavior

### Integration Tests
- Test command combinations
- Test error handling and edge cases
- Test signal behavior

### Regression Tests
- Keep every test that found a bug
- Run full test suite before each merge to main
