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
void parser_free_ast(t_ast *ast);

// Collect heredoc content after parsing (walk AST, read from input)
int parser_collect_heredocs(t_shell *shell, t_ast *ast);
```

## Shell Grammar (Simplified POSIX)

```
complete_command : list
                 | list '&'
                 | list ';'
                 ;

list             : list ';' and_or
                 | and_or
                 ;

and_or           : and_or '&&' pipeline
                 | and_or '||' pipeline
                 | pipeline
                 ;

pipeline         : pipeline '|' command
                 | command
                 ;

command          : simple_command
                 | '(' list ')'  redirect_list?
                 | '{' list '}'  redirect_list?
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
```

## Operator Precedence (Low to High)

```
1. ;     (sequence)      - lowest precedence
2. && || (and/or)        - left-to-right associativity
3. |     (pipe)          - left-to-right associativity
4. command               - highest precedence
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
parse_complete_command(p):
    node = parse_list(p)
    if current is '&':
        advance
        node = new background node wrapping node
    else if current is ';':
        advance     # trailing ; is ok
    if current is not TOK_EOF:
        syntax error
    return node

parse_list(p):
    left = parse_and_or(p)
    while accept(';'):
        if current is EOF or ')' or '}':
            break       # trailing ; before end
        right = parse_and_or(p)
        left = new SEQUENCE node(left, right)
    return left

parse_and_or(p):
    left = parse_pipeline(p)
    while current is '&&' or '||':
        op = current type
        advance
        right = parse_pipeline(p)
        left = new binary node(op, left, right)
    return left

parse_pipeline(p):
    left = parse_command(p)
    while accept('|'):
        right = parse_command(p)
        left = new PIPE node(left, right)
    return left

parse_command(p):
    if accept('('):
        inner = parse_list(p)
        expect(')')
        redirs = parse_redirect_list(p)    # optional redirections after )
        return new SUBSHELL node(inner, redirs)

    if accept('{'):
        inner = parse_list(p)
        expect('}')
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
