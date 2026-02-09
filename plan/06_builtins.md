# Builtins Module

## Purpose

Implement shell built-in commands. Builtins run in the shell process itself (not forked) because they need to modify shell state. **Exception:** in pipelines, builtins run in forked children (the pipeline code handles this).

## Interface

```c
// Builtin function type
typedef int (*t_builtin_fn)(t_shell *shell, int argc, char **argv);

// Get builtin function by name (NULL if not a builtin)
t_builtin_fn builtin_get(const char *name);

// Check if command is a builtin
int builtin_is_builtin(const char *name);
```

## Builtin Registry

Use a static lookup table mapping names to function pointers:

```c
static const struct { const char *name; t_builtin_fn fn; } g_builtins[] = {
    // Mandatory
    {"cd",      builtin_cd},
    {"echo",    builtin_echo},
    {"exit",    builtin_exit},
    {"type",    builtin_type},
    {"export",  builtin_export},
    {"unset",   builtin_unset},
    {"set",     builtin_set},
    {"jobs",    builtin_jobs},
    {"fg",      builtin_fg},
    {"bg",      builtin_bg},
    // Modular
    {"alias",   builtin_alias},
    {"unalias", builtin_unalias},
    {"hash",    builtin_hash},
    {"test",    builtin_test},
    {"[",       builtin_test},
    {"fc",      builtin_fc},
    {NULL, NULL}
};
```

`builtin_get(name)` iterates the table and returns the matching function pointer or NULL.

---

## Mandatory Builtins

### cd

**Synopsis:** `cd [-L|-P] [directory]`

**Behavior:**
- Parse `-L` (logical, default) and `-P` (physical) options
- No argument: go to `$HOME`, error if unset
- Argument is `-`: go to `$OLDPWD`, error if unset, print new directory
- Otherwise: chdir to argument
- Before changing: save current dir in `$OLDPWD`
- After changing: update `$PWD` with getcwd

**Edge cases:**
- `cd ""` → error "No such file or directory" (bash behavior)
- `cd` with `HOME` unset → error
- Multiple `-L`/`-P` flags: last one wins
- `--` stops option parsing

---

### echo

**Synopsis:** `echo [-n] [-e] [-E] [string ...]`

**Behavior:**
- Parse flags at the beginning only: `-n` (no newline), `-e` (interpret escapes), `-E` (don't interpret, default)
- Combined flags ok: `echo -neE` → `-n` active, `-E` wins over `-e`
- A flag argument must contain ONLY `n`, `e`, `E` chars (after `-`). If invalid, treat as text.
- Print all remaining arguments separated by spaces
- Print newline at end unless `-n`
- With `-e`: interpret `\n`, `\t`, `\\`, `\a`, `\b`, `\f`, `\r`, `\v`, `\0nnn`

**Edge cases:**
- `echo -` → prints `-`
- `echo -nnn` → no output, no newline (valid flag)
- `echo -nq` → prints `-nq` (invalid flag char `q`)

---

### exit

**Synopsis:** `exit [n]`

**Behavior:**
- No argument: exit with `$?` (last exit status)
- One argument: must be numeric, exit with `n & 0xFF`
- Non-numeric argument: print error, exit with status 2
- More than one argument: print "too many arguments", do NOT exit, return 1
- If there are stopped jobs: print warning, set `exit_confirmed` flag, don't exit on first attempt. Exit on second consecutive `exit`.
- Print "exit" to stderr before exiting

---

### type

**Synopsis:** `type name [name ...]`

**Behavior:**
- For each name, check in order:
  1. Alias → `name is aliased to 'value'`
  2. Builtin → `name is a shell builtin`
  3. PATH search → `name is /full/path`
  4. Not found → `type: name: not found` (to stderr), return 1
- Return 0 if all found, 1 if any not found

---

### export

**Synopsis:** `export [-p] [name[=value] ...]`

**Behavior:**
- No arguments or `-p`: print all exported variables as `declare -x NAME="VALUE"`
- `export NAME`: mark existing variable as exported (create with null value if doesn't exist)
- `export NAME=value`: set value AND mark as exported
- Validate identifier (alphanumeric + underscore, starts with letter/underscore)
- Invalid identifier: print error, return 1, continue processing others

---

### unset

**Synopsis:** `unset name [name ...]`

**Behavior:**
- Remove each named variable from shell
- Invalid identifier: print error, continue processing
- Unsetting a non-existent variable is not an error
- Always return 0 (unless invalid identifier)

---

### set

**Synopsis:** `set` (no options required for mandatory)

**Behavior:**
- No arguments: print all shell variables as `NAME=VALUE`, one per line
- Sorted alphabetically (bash behavior)

---

### jobs

**Synopsis:** `jobs [-l] [-p]`

**Behavior:**
- No flags: print `[id]± status command` for each job
- `-l`: include PID in output
- `-p`: print only PGIDs
- Before printing: update job statuses (non-blocking waitpid)
- `+` marks current job, `-` marks previous job

---

### fg

**Synopsis:** `fg [job_spec]`

**Behavior:**
- No argument: bring current job (`%+`) to foreground
- Parse job spec: `%1` (by id), `%+` (current), `%-` (previous), `%string` (by prefix)
- Print the command line
- Send SIGCONT if job was stopped
- Give job's process group the terminal (tcsetpgrp)
- Wait for job to complete or stop
- Take terminal back for shell

---

### bg

**Synopsis:** `bg [job_spec]`

**Behavior:**
- No argument: continue current job in background
- Job must be stopped (error if running)
- Send SIGCONT to job's process group
- Print `[id]+ command &`
- Return immediately (don't wait)

---

## Modular Builtins

### alias / unalias

**alias:** No args prints all. `alias name=value` sets. `alias name` prints one.

**unalias:** `unalias name` removes. `unalias -a` removes all.

Aliases are expanded before other processing: the first word of a command is checked against the alias list, and if found, replaced with the alias value (which may contain multiple words, requiring re-tokenization).

### test / [

Evaluate conditional expressions. The `[` form requires `]` as last argument.

Operators: file tests (`-d`, `-f`, `-e`, `-r`, `-w`, `-x`, etc.), string tests (`-z`, `=`, `!=`), numeric tests (`-eq`, `-ne`, `-gt`, `-lt`, `-ge`, `-le`), logical (`!`).

### hash

`hash` lists cached command paths. `hash -r` clears cache. `hash name` adds to cache. Speeds up PATH lookups.

### fc

History manipulation: `fc -l` lists, `fc -s` re-executes, `fc` opens editor.

---

## Files

```
src/builtins/
├── builtins.c        # Registry and lookup
├── builtin_cd.c
├── builtin_echo.c
├── builtin_exit.c
├── builtin_type.c
├── builtin_export.c
├── builtin_unset.c
├── builtin_set.c
├── builtin_jobs.c
├── builtin_fg.c
├── builtin_bg.c
├── builtin_alias.c   # Modular
├── builtin_hash.c    # Modular
├── builtin_test.c    # Modular
└── builtin_fc.c      # Modular
```
