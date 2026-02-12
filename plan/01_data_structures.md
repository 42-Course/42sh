# Core Data Structures

## 1. Token (Lexer Output)

```c
typedef enum e_token_type
{
    // Words
    TOK_WORD,               // Regular word/argument (quotes preserved in value)

    // Operators
    TOK_PIPE,               // |
    TOK_AND,                // &&
    TOK_OR,                 // ||
    TOK_SEMICOLON,          // ;
    TOK_AMPERSAND,          // & (background)
    TOK_NEWLINE,            // \n

    // Redirections
    TOK_REDIR_IN,           // <
    TOK_REDIR_OUT,          // >
    TOK_REDIR_APPEND,       // >>
    TOK_HEREDOC,            // <<
    TOK_REDIR_DUP_IN,       // <&
    TOK_REDIR_DUP_OUT,      // >&

    // Grouping (modular)
    TOK_LPAREN,             // (   — operator, breaks words
    TOK_RPAREN,             // )   — operator, breaks words
    // NOTE: { and } are NOT token types. They are reserved words:
    // the lexer produces TOK_WORD with value "{" or "}".
    // The parser recognizes them in command position.

    // Special
    TOK_EOF,                // End of input
    TOK_ERROR               // Lexer error
}   t_token_type;

typedef struct s_token
{
    t_token_type    type;
    char            *value;         // Raw token string (quotes preserved for expander)
    int             io_number;      // FD number before redirect (-1 if none)
    struct s_token  *next;
}   t_token;
```

**Design decision:** No `TOK_ASSIGNMENT` type. Assignment detection (`VAR=value`) is done by the parser in the command prefix position, not by the lexer. This avoids the bug where `echo VAR=value` would be misclassified.

**Design decision:** Quotes are preserved in `value`. The string `"hello"'world'$VAR` is stored as-is. The expander handles quote interpretation during expansion. This allows the expander to know which parts are single-quoted (no expansion), double-quoted (partial expansion), or unquoted (full expansion).

## 2. AST Nodes (Parser Output)

### Union-based approach

```c
typedef enum e_node_type
{
    NODE_COMMAND,           // Simple command: ls -la
    NODE_PIPE,              // Pipe: cmd1 | cmd2
    NODE_AND,               // And: cmd1 && cmd2
    NODE_OR,                // Or: cmd1 || cmd2
    NODE_SEQUENCE,          // Sequence: cmd1 ; cmd2
    NODE_SUBSHELL,          // Subshell: (cmd)
    NODE_BLOCK,             // Block: { cmd; }
    NODE_BACKGROUND,        // Background: cmd &
}   t_node_type;

// Redirection
typedef struct s_redir
{
    t_token_type    type;           // Type of redirection
    int             fd;             // File descriptor (default: -1 = use default)
    char            *target;        // Filename or fd number (raw, unexpanded)
    char            *heredoc_delim;     // For heredoc: the delimiter word
    char            *heredoc_content;   // For heredoc: collected content (filled after parse)
    int             heredoc_quoted;     // Was delimiter quoted? (no expansion if yes)
    struct s_redir  *next;
}   t_redir;

// Simple command
typedef struct s_cmd
{
    char            **argv;         // Arguments (argv[0] is command), raw/unexpanded
    int             argc;           // Argument count
    char            **assignments;  // VAR=value before command (detected by parser)
    t_redir         *redirs;        // Redirections
}   t_cmd;

// Binary operation (pipe, &&, ||, ;)
typedef struct s_binary
{
    struct s_ast    *left;
    struct s_ast    *right;
}   t_binary;

// Group (subshell, block, background) - can have own redirections
typedef struct s_group
{
    struct s_ast    *child;
    t_redir         *redirs;        // Redirections on the group: (cmd) > file
}   t_group;

// AST Node
typedef struct s_ast
{
    t_node_type     type;
    union {
        t_cmd       cmd;        // NODE_COMMAND
        t_binary    binary;     // NODE_PIPE, NODE_AND, NODE_OR, NODE_SEQUENCE
        t_group     group;      // NODE_SUBSHELL, NODE_BLOCK, NODE_BACKGROUND
    } data;
}   t_ast;
```

**Note:** All strings in the AST (argv, assignments, redir targets) are raw/unexpanded. The expander processes them at execution time, not at parse time.

## 3. Variable Storage

```c
typedef struct s_var
{
    char            *name;
    char            *value;
    int             exported;       // Is in environment?
    int             readonly;       // Is read-only? (bonus)
    struct s_var    *next;
}   t_var;
```

Alternatively, use a hash table for O(1) lookup if the variable count is large:

```c
typedef struct s_var_table
{
    t_var           **buckets;
    size_t          size;
    size_t          count;
}   t_var_table;
```

## 4. Job Control

```c
typedef enum e_job_status
{
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE,
    JOB_TERMINATED
}   t_job_status;

typedef struct s_process
{
    pid_t           pid;
    char            *cmd;           // Command string for display
    int             status;         // waitpid status
    int             completed;
    int             stopped;
    struct s_process *next;
}   t_process;

typedef struct s_job
{
    int             id;             // Job number [1], [2], etc.
    pid_t           pgid;           // Process group ID
    char            *cmd_line;      // Full command line
    t_process       *processes;     // Processes in this job
    t_job_status    status;
    int             notified;       // User notified of status?
    int             foreground;     // Was started in foreground?
    struct s_job    *next;
}   t_job;
```

## 5. History

```c
typedef struct s_history_entry
{
    int             number;         // History number
    char            *line;          // Command line
    struct s_history_entry *prev;
    struct s_history_entry *next;
}   t_history_entry;

typedef struct s_history
{
    t_history_entry *head;          // Oldest
    t_history_entry *tail;          // Newest
    t_history_entry *current;       // For navigation
    int             count;
    int             max_size;       // Max entries to keep
    char            *file_path;     // ~/.42sh_history
}   t_history;
```

## 6. Line Editor State

```c
typedef struct s_line_editor
{
    // Buffer
    char            *buffer;        // Input buffer
    size_t          buf_size;       // Allocated size
    size_t          len;            // Current length
    size_t          cursor;         // Cursor position

    // Display
    size_t          prompt_len;     // Prompt length
    int             term_cols;      // Terminal width
    int             term_rows;      // Terminal height

    // Terminal
    struct termios  orig_termios;   // Original settings
    int             raw_mode;       // In raw mode?

    // History
    t_history       *history;
    char            *saved_line;    // Line before history nav

    // Multi-line
    int             in_quote;       // Unclosed quote type
    int             line_continuation; // Backslash at EOL
}   t_line_editor;
```

## 7. Alias (Modular Feature)

```c
typedef struct s_alias
{
    char            *name;
    char            *value;
    struct s_alias  *next;
}   t_alias;
```

## 8. Hash Table for Command Cache (Modular Feature)

```c
typedef struct s_hash_entry
{
    char            *name;          // Command name
    char            *path;          // Full path
    int             hits;           // Usage count
    struct s_hash_entry *next;
}   t_hash_entry;

typedef struct s_hash_table
{
    t_hash_entry    **buckets;
    size_t          size;
}   t_hash_table;
```

## Memory Management Guidelines

1. **Tokens**: Free after parsing (parser consumes them)
2. **AST**: Free after execution
3. **Variables**: Free when unset or shell exits
4. **Jobs**: Free when job completes and is reported
5. **History**: Free on shell exit, save to file first

### Helper functions to implement:

```c
// Token
t_token     *token_new(t_token_type type, char *value);
void        token_free(t_token *token);
void        token_list_free(t_token *head);

// AST
t_ast       *ast_new_command(t_cmd *cmd);
t_ast       *ast_new_binary(t_node_type type, t_ast *left, t_ast *right);
t_ast       *ast_new_group(t_node_type type, t_ast *child, t_redir *redirs);
void        ast_free(t_ast *node);

// Redirection
t_redir     *redir_new(t_token_type type, char *target);
void        redir_free(t_redir *redir);
void        redir_list_free(t_redir *head);

// Variables
t_var       *var_get(t_shell *shell, const char *name);
int         var_set(t_shell *shell, const char *name, const char *value);
int         var_unset(t_shell *shell, const char *name);
int         var_export(t_shell *shell, const char *name);
```
