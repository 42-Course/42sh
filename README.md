# 42sh Architecture Plan

## Document Index

| Document | Description |
|----------|-------------|
| [00_overview.md](00_overview.md) | High-level architecture, data flow, file organization |
| [01_data_structures.md](01_data_structures.md) | Core data structures (tokens, AST, jobs, etc.) |
| [02_lexer.md](02_lexer.md) | Lexer module: tokenization, state machine |
| [03_parser.md](03_parser.md) | Parser module: grammar, recursive descent |
| [04_expander.md](04_expander.md) | Expander module: variable/tilde/glob expansion |
| [05_executor.md](05_executor.md) | Executor module: command execution, pipes, redirections |
| [06_builtins.md](06_builtins.md) | Builtin commands implementation |
| [07_line_editor.md](07_line_editor.md) | Line editor: termcap, key handling, history |
| [08_job_control.md](08_job_control.md) | Job control: process groups, fg/bg |
| [09_signals.md](09_signals.md) | Signal handling |
| [10_variables.md](10_variables.md) | Variable management |
| [11_team_assignment.md](11_team_assignment.md) | Team roles and responsibilities |
| [12_development_phases.md](12_development_phases.md) | Development order and milestones |
| [13_modular_features.md](13_modular_features.md) | Guide to optional features |

## Quick Start

### 1. Understand the Architecture
Read `00_overview.md` to understand how modules connect.

### 2. Agree on Data Structures
Review `01_data_structures.md` as a team. These structures are shared.

### 3. Assign Roles
Use `11_team_assignment.md` to divide work.

### 4. Follow Development Phases
Use `12_development_phases.md` as your roadmap.

### 5. Choose Modular Features
Use `13_modular_features.md` to pick your 6 features.

## Architecture Summary

```
┌──────────────────────────────────────────────────────────────┐
│                         Main Loop                            │
│   while (running) {                                          │
│       line = line_editor_readline(prompt);                   │
│       tokens = lexer_tokenize(line);                         │
│       ast = parser_parse(tokens);                            │
│       expander_expand(ast);                                  │
│       status = executor_execute(ast);                        │
│   }                                                          │
└──────────────────────────────────────────────────────────────┘
```

## Key Principles

1. **Stability First** - Never crash, always handle errors
2. **Clear Interfaces** - Modules communicate through defined functions
3. **Test Early** - Have something working at each phase
4. **Reference Bash** - When in doubt, check bash behavior

## File Structure

```
42sh/
├── Makefile
├── include/
│   ├── shell.h
│   ├── lexer.h
│   ├── parser.h
│   ├── ast.h
│   ├── expander.h
│   ├── executor.h
│   ├── builtins.h
│   ├── variables.h
│   ├── job_control.h
│   ├── signals.h
│   ├── line_editor.h
│   └── utils.h
├── src/
│   ├── main.c
│   ├── lexer/
│   ├── parser/
│   ├── expander/
│   ├── executor/
│   ├── builtins/
│   ├── variables/
│   ├── job_control/
│   ├── signals/
│   ├── line_editor/
│   └── utils/
├── libft/
├── tests/
└── plan/
```

## Team Contacts

| Role | Person | Modules |
|------|--------|---------|
| Parsing | TBD | Lexer, Parser |
| Execution | TBD | Executor, Redirections |
| Expansion | TBD | Expander, Variables |
| Terminal | TBD | Line Editor, History |
| System | TBD | Job Control, Signals, Builtins |
