# Parser Module

## Purpose

Convert a list of tokens into an Abstract Syntax Tree (AST). The parser handles:

- Operator precedence
- Command grouping
- Syntax validation
- Assignment detection (in command prefix position)
- Building the tree structure

## Interface

```c
// Main parsing function - returns AST or NULL on syntax error
t_ast *parser_parse(t_token *tokens);

// Free AST
void ast_free(t_ast *ast);

// Collect heredoc content after parsing (walk AST, read from input)
int parser_collect_heredocs(t_shell *shell, t_ast *ast);
```

## Shell Grammar (Simplified POSIX)

```
complete_command : list
                 ;

list             : list separator and_or
                 | and_or
                 ;

separator        : ';'                            /* sequence */
                 | '&'                            /* background the preceding and_or */
                 | NEWLINE                        /* same as ; */
                 ;

and_or           : and_or '&&' newlines pipeline
                 | and_or '||' newlines pipeline
                 | pipeline
                 ;

pipeline         : pipeline '|' newlines command
                 | command
                 ;

command          : simple_command
                 | '(' list ')'  redirect_list?
                 | WORD<'{'>  list WORD<'}'> redirect_list?
                 ;

simple_command   : cmd_prefix cmd_word cmd_suffix
                 | cmd_prefix cmd_word
                 | cmd_prefix                     /* assignments only */
                 | cmd_name cmd_suffix
                 | cmd_name
                 ;

cmd_prefix       : io_redirect
                 | cmd_prefix io_redirect
                 | ASSIGNMENT_WORD                /* detected by parser, not lexer */
                 | cmd_prefix ASSIGNMENT_WORD
                 ;

cmd_suffix       : io_redirect
                 | cmd_suffix io_redirect
                 | WORD
                 | cmd_suffix WORD
                 ;

io_redirect      : IO_NUMBER? io_file
                 | IO_NUMBER? '<<' WORD
                 ;

io_file          : '<' WORD
                 | '>' WORD
                 | '>>' WORD
                 | '<&' WORD
                 | '>&' WORD
                 ;

newlines         : NEWLINE*                       /* skip any newlines (continuation) */
                 ;
```

**Key grammar changes vs naive approach:**
- `&` is a **separator** at the same level as `;` and NEWLINE. It backgrounds the and_or to its left. This makes `cmd1 & cmd2` work correctly.
- **NEWLINE** is treated as a separator (like `;`). After `|`, `&&`, `||`, newlines are allowed (continuation lines, e.g., `ls |\n  grep foo`).
- `{` and `}` are **reserved words** (WORD tokens with value `{`/`}`), NOT operators. See lexer notes.

## Operator Precedence (Low to High)

```
1. ; & \n (list separators)  - lowest precedence
2. && ||  (and/or)           - left-to-right associativity
3. |      (pipe)             - left-to-right associativity
4. command                   - highest precedence
```

Example: `a ; b && c | d || e`
Parses as: `a ; ((b && (c | d)) || e)`

```
        SEQUENCE
        /      \
       a        OR
              /    \
            AND     e
           /   \
          b    PIPE
              /    \
             c      d
```

Example: `a & b && c`
Parses as: `SEQUENCE(BACKGROUND(a), (b && c))`

```
        SEQUENCE
        /      \
   BACKGROUND   AND
       |       /   \
       a      b     c
```

## Assignment Detection (by Parser)

A word is an assignment if ALL of these are true:
1. It contains `=`
2. The part before `=` is a valid identifier (starts with letter/`_`, rest alphanumeric/`_`)
3. It appears in the **cmd_prefix** position (before the command name)

After the command name, ALL words are arguments, even if they look like assignments.

```bash
VAR=value cmd      # VAR=value is assignment (prefix position)
cmd VAR=value      # VAR=value is argument (suffix position)
=value             # argument (no valid name before =)
1VAR=value         # argument (name starts with digit)
```

Pseudo code:
```
is_assignment_word(word_value):
    find '=' in word_value
    if no '=' found: return false
    name_part = substring before '='
    if name_part is empty: return false
    if name_part[0] is not letter and not '_': return false
    for each char in name_part:
        if not alphanumeric and not '_': return false
    return true
```

## Recursive Descent Parser

Each grammar rule becomes a function:

```
parse_complete_command(p)  → Entry point
parse_list(p)              → Handle ;
parse_and_or(p)            → Handle && ||
parse_pipeline(p)          → Handle |
parse_command(p)           → Handle command/subshell/block
parse_simple_command(p)    → Handle simple command
parse_io_redirect(p)       → Handle redirections
```

### Parser State

```c
typedef struct s_parser
{
    t_token     *tokens;        // Token list
    t_token     *current;       // Current token
    char        *error;         // Error message if any
}   t_parser;
```

Helper operations:
- `advance(p)` → move to next token
- `accept(p, type)` → if current matches type, advance and return true
- `expect(p, type)` → accept or set syntax error

### Pseudo Code

```
skip_newlines(p):
    while current is TOK_NEWLINE:
        advance

parse_complete_command(p):
    skip_newlines(p)                # skip leading newlines
    if current is TOK_EOF:
        return NULL                 # empty input
    node = parse_list(p)
    if current is not TOK_EOF:
        syntax error
    return node

parse_list(p):
    left = parse_and_or(p)

    while current is ';' or '&' or NEWLINE:
        sep = current type
        advance
        skip_newlines(p)            # consume consecutive separators/newlines

        # '&' backgrounds the command/pipeline BEFORE it
        if sep == '&':
            left = background_last(left)

        # Trailing separator with nothing after?
        if current is EOF or ')' or WORD("}"):
            break

        right = parse_and_or(p)
        left = new SEQUENCE(left, right)

    return left

# Helper: when & appears, background only the rightmost pipeline in
# the accumulating tree, not the entire sequence built so far.
background_last(node):
    if node is SEQUENCE:
        node->right = new BACKGROUND(node->right)
        return node
    return new BACKGROUND(node)

parse_and_or(p):
    left = parse_pipeline(p)
    while current is '&&' or '||':
        op = current type
        advance
        skip_newlines(p)            # allow continuation: cmd1 && \n cmd2
        right = parse_pipeline(p)
        left = new binary node(op, left, right)
    return left

parse_pipeline(p):
    left = parse_command(p)
    while accept('|'):
        skip_newlines(p)            # allow continuation: ls | \n grep foo
        right = parse_command(p)
        left = new PIPE node(left, right)
    return left

parse_command(p):
    if accept('('):                 # modular: subshell
        inner = parse_list(p)
        expect(')')
        redirs = parse_redirect_list(p)
        return new SUBSHELL node(inner, redirs)

    if current is WORD and value == '{':    # modular: block (reserved word)
        advance
        inner = parse_list(p)
        expect WORD with value '}'
        redirs = parse_redirect_list(p)
        return new BLOCK node(inner, redirs)

    return parse_simple_command(p)

parse_simple_command(p):
    args = empty list
    assignments = empty list
    redirs = empty list
    found_cmd_name = false

    # Parse prefix: redirections and assignments before command name
    while true:
        if current is a redirect token:
            parse_io_redirect into redirs
        else if current is WORD and not found_cmd_name and is_assignment_word(value):
            add value to assignments
            advance
        else:
            break

    # Parse command name + suffix
    while current is WORD:
        found_cmd_name = true
        add value to args
        advance
        # Suffix redirections
        while current is a redirect token:
            parse_io_redirect into redirs

    # Must have at least something
    if args is empty and assignments is empty and redirs is empty:
        syntax error
        return NULL

    return new COMMAND node(args, assignments, redirs)

parse_io_redirect(p):
    type = current redirect type
    io_number = current token's io_number field
    advance                     # consume redirect operator
    if current is not WORD:
        syntax error "expected filename after redirection"
        return NULL
    target = current value
    advance
    return new redir(type, io_number, target)
```

## Heredoc Collection

After `parser_parse()` returns the AST, collect all heredoc content **before** execution begins. This is a separate pass because heredocs must be read in the order they appear, and ALL must be read before any command runs.

```
parser_collect_heredocs(shell, ast):
    walk the AST depth-first
    for each NODE_COMMAND found:
        for each redir in command's redir list:
            if redir type is TOK_HEREDOC:
                redir.heredoc_quoted = (delimiter contains any quotes)
                redir.heredoc_delim = remove quotes from delimiter
                read lines from input until delimiter line is found
                store concatenated lines in redir.heredoc_content
                if interrupted by signal: return error
```

Example:
```bash
cat << EOF ; echo hi
hello
world
EOF
```
After parsing, we get AST with `cat << EOF` and `echo hi`. Before executing either, we read lines `hello\nworld\n` and store in the heredoc redir's `heredoc_content` field.

## Syntax Error Handling

Error messages should match bash format:
```
42sh: syntax error near unexpected token `|'
42sh: syntax error near unexpected token `newline'
42sh: syntax error: unexpected end of file
```

Strategy:
- Keep first error only (don't cascade)
- On error, return NULL from parser_parse
- Caller sets `$?` to 2 (syntax error convention)

## Edge Cases

1. **Empty input**: Return NULL (no command)
2. **Just semicolons**: `;;;` → skip empty commands
3. **Trailing operators**: `ls |` → syntax error
4. **Leading operators**: `| ls` → syntax error
5. **Double operators**: `ls && && echo` → syntax error
6. **Unclosed parens**: `(ls` → syntax error
7. **Empty subshell**: `()` → syntax error in bash
8. **Redirections only**: `> file` → valid (null command with redirect in bash)
9. **Assignments only**: `VAR=value` → valid (set variable, no command)
10. **Mixed**: `VAR=value > file cmd arg` → all valid

## Testing

```bash
# Valid
ls
ls -la
ls | grep foo
ls && echo ok
ls || echo fail
ls ; pwd
(ls ; pwd) | grep foo
(ls) > file
ls > file
ls >> file 2>&1
VAR=value cmd
< input cmd > output
VAR=value
sleep 10 & echo done          # & as separator: sleep in bg, echo in fg
cmd1 & cmd2 & cmd3            # cmd1 bg, cmd2 bg, cmd3 fg
cmd1 & cmd2 &                 # both backgrounded
ls |
  grep foo                    # continuation after | (newline)
true &&
  echo ok                     # continuation after && (newline)

# Invalid
|
ls |
ls &&
ls || &&
(ls
ls )
> < file
```

## Files

```
src/parser/
├── parser.c              # Main parse function, parser state
├── parser_list.c         # parse_list, parse_and_or
├── parser_pipeline.c     # parse_pipeline
├── parser_command.c      # parse_command, parse_simple_command
├── parser_redirect.c     # parse_io_redirect
├── parser_heredoc.c      # Heredoc collection pass
├── parser_utils.c        # Helpers (advance, accept, expect, is_assignment)
└── ast.c                 # AST node creation/deletion
```
