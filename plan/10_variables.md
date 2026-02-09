# Variables Module

## Purpose

Manage shell variables (internal and environment). This module handles:

- Internal variable storage (`name=value`)
- Environment variable management
- Variable lookup for expansion
- Export/unexport functionality
- Environment array for `execve()`

## Types of Variables

### Internal Variables
- Stored only in shell
- Created with `name=value` (no command following)
- Not visible to child processes

### Environment Variables
- Inherited from parent process on startup
- Passed to child processes via `execve()`
- Listed by `env` command

### Exported Variables
- Internal variables marked for export
- Become environment variables for children
- Created via `export NAME` or `export NAME=value`

## Data Structure

```c
typedef struct s_var
{
    char            *name;
    char            *value;
    int             exported;   // Should be in environment?
    struct s_var    *next;
}   t_var;
```

In `t_shell`:
```c
t_var   *variables;     // Linked list of all variables
char    **env;          // Cached environment array (for execve)
int     env_dirty;      // Needs rebuild?
```

Alternatively, use a hash table for O(1) lookup (especially if implementing the hash table modular feature).

## Interface

```c
char    *var_get_value(t_shell *shell, const char *name);
t_var   *var_get(t_shell *shell, const char *name);
int     var_set(t_shell *shell, const char *name, const char *value);
int     var_unset(t_shell *shell, const char *name);
int     var_export(t_shell *shell, const char *name);
char    **var_get_environ(t_shell *shell);
void    var_init_from_environ(t_shell *shell, char **environ);
```

## Initialization

```
var_init_from_environ(shell, environ):
    for each string in environ:
        split on first '='
        var_set(name, value)
        var_export(name)

    set PWD = getcwd()
    increment SHLVL (or set to 1 if unset)
    set $0 = "42sh" (or argv[0])
```

## Core Operations

### Get

```
var_get_value(shell, name):
    walk linked list, compare names
    return value if found, NULL otherwise

var_get(shell, name):
    same but returns the node (for checking exported flag)
```

### Set

```
var_set(shell, name, value):
    validate name (must be valid identifier)
    if variable exists: update its value
    else: create new node, add to list
    mark env_dirty = 1
```

### Unset

```
var_unset(shell, name):
    find variable in list
    if found: unlink from list, free node
    mark env_dirty = 1
    (unsetting non-existent var is not an error)
```

### Export

```
var_export(shell, name):
    if variable exists: set exported = 1
    else: create variable with NULL value, set exported = 1
    mark env_dirty = 1
```

### Identifier Validation

```
is_valid_identifier(name):
    if empty: false
    if first char not letter and not '_': false
    for remaining chars: if not alphanumeric and not '_': false
    return true
```

## Environment Array for execve()

`execve()` needs a `char **` array. We cache this and rebuild lazily:

```
var_get_environ(shell):
    if not env_dirty: return cached env

    free old env array
    count exported variables with non-NULL values
    allocate new array of "NAME=VALUE" strings
    for each exported variable with value:
        env[i] = "name=value"
    env[count] = NULL

    env_dirty = 0
    return env
```

**Important:** The executor must call `var_get_environ(shell)` before `execve()`, not use `shell->env` directly, to ensure the array is up-to-date.

## Special Variables

### Auto-set by Shell
- `PWD` - updated by `cd`
- `OLDPWD` - updated by `cd`
- `SHLVL` - incremented on shell start
- `$?` - last exit status (not stored in variable list; accessed via `shell->last_exit_status`)
- `$$` - shell PID (accessed via `getpid()`)
- `$0` - shell name (set at init)

### Read-only (bonus)
Some variables like `$$` and `$0` should not be modifiable by the user.

## Assignment Handling

For `VAR=value command` (assignments before a command):

- If **no command follows**: assignments are persistent in current shell
- If **command follows**: assignments are temporary, apply only for that command's environment

```
Temporary assignment flow:
    1. Save current values of assigned variables
    2. Apply new values and mark as exported
    3. Execute the command
    4. Restore original values (or unset if didn't exist)
```

This is handled by the executor, not the variables module, but uses the variables interface.

## Files

```
src/variables/
├── variables.c       # Core: get, set, unset, export
├── var_environ.c     # Environment array building
├── var_init.c        # Initialization from environ
└── var_utils.c       # Helpers: identifier validation, etc.
```
