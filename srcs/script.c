/**
 * @file script.c
 * @brief Non-interactive input drivers: script files, `source`, and the
 *        startup rc file.
 * @author pulgamecanica
 *
 * All three entry points funnel through @c shell_run_stream, which reuses the
 * REPL's own readers (`shell_read_logical_line`) and executor so that
 * multi-line constructs, heredocs and aliases behave exactly as they do
 * interactively. The only differences are that no prompt is printed, history
 * is not recorded, and EOF on the stream simply ends the run instead of
 * exiting the shell.
 */

#include "42sh.h"
#include "lexer.h"
#include "parser.h"
#include "executor.h"
#include <string.h>

#ifdef FT_EXTRA_VERBOSE
static void	_display(t_list *tokens, t_ast *ast, char *line)
{
	char	*tok_json;
	char	*ast_json;

	lexer_display(tokens, line);
	tok_json = lexer_to_json(tokens, line);
	if (tok_json)
		printf("  \033[2mJSON → %s\033[0m\n", tok_json);
	ast_display(ast, line);
	ast_json = ast_to_json(ast, line, tok_json);
	if (ast_json)
	{
		printf("  \033[2mAST  → %s\033[0m\n", ast_json);
		free(ast_json);
	}
	free(tok_json);
}
#endif

void	process_line(t_shell *shell, char *line)
{
	t_list		*tokens;
	t_ast		*ast;
	const char	*scan;

	scan = line;
	while (*scan == ' ' || *scan == '\t' || *scan == '\n')
		scan++;
	if (*scan == '\0')
		return ;
	tokens = lexer_tokenize(line);
	if (!tokens)
	{
		shell->last_exit_status = 1;
		return ;
	}
	if (((t_token *)tokens->content)->type == TOK_EOF)
		return (lexer_free_tokens(tokens));
	alias_expand_tokens(shell, &tokens);
	ast = parser_parse(tokens, shell);
#ifdef FT_EXTRA_VERBOSE
	_display(tokens, ast, line);
#endif
	lexer_free_tokens(tokens);
	if (!ast)
		return ;
	executor_execute(shell, ast);
	ast_free(ast);
}

int	shell_run_stream(t_shell *shell, FILE *f)
{
	FILE	*saved;
	char	*raw;
	char	*line;

	saved = shell->input;
	shell->input = f;
	while (shell->running)
	{
		raw = shell_read_logical_line(shell, "");
		if (!raw)
			break ;
		line = ft_strtrim(raw);
		free(raw);
		if (*line == '\0')
		{
			free(line);
			continue ;
		}
		process_line(shell, line);
		free(line);
	}
	shell->input = saved;
	return (shell->last_exit_status);
}

int	shell_run_script(t_shell *shell, const char *path)
{
	FILE	*f;

	f = fopen(path, "r");
	if (!f)
	{
		ft_putstr_fd("42sh: ", 2);
		ft_putstr_fd((char *)path, 2);
		ft_putstr_fd(": ", 2);
		ft_putendl_fd(strerror(errno), 2);
		shell->last_exit_status = 127;
		return (127);
	}
	shell_run_stream(shell, f);
	fclose(f);
	return (shell->last_exit_status);
}

/**
 * @brief Resolve the rc file path: @c $ENV if set, else @c $HOME/.42shrc.
 * @return Path to source, or NULL when neither is available. The returned
 *         pointer is either owned by the shell's variable table ($ENV case,
 *         do not free) or freshly allocated ($HOME case, caller frees) — the
 *         @p owned flag tells which.
 */
static char	*resolve_rc_path(t_shell *shell, int *owned)
{
	char	*env;
	char	*home;

	*owned = 0;
	env = var_get_value(shell, "ENV");
	if (env && *env)
		return (env);
	home = var_get_value(shell, "HOME");
	if (!home || !*home)
		return (NULL);
	*owned = 1;
	return (ft_strjoin(home, "/.42shrc"));
}

void	shell_source_rc(t_shell *shell)
{
	char	*path;
	int		owned;
	FILE	*f;

	path = resolve_rc_path(shell, &owned);
	if (!path)
		return ;
	f = fopen(path, "r");
	if (f)
	{
		shell_run_stream(shell, f);
		fclose(f);
	}
	if (owned)
		free(path);
}
