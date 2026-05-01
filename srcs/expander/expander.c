/**
 * @file expander.c
 * @brief Word expansion functions for the 42sh shell.
 *
 * @author pulgamecanica wengzhang jspitz
*/

#include "42sh.h"
#include "expander.h"
#include "ast.h"

char *expand_word(struct s_shell *shell, const char *word) {
	
	char *expanded = expand_dollar(shell, (char *)word, 0, false);
	
	char *final_expanded = expand_tilde(shell, expanded, 0);
	
	free(expanded);
	
	return final_expanded;
}

int expand_command(struct s_shell *shell, t_cmd *cmd) {
	if (!cmd) {
		return 0;
	}

	for (size_t i = 0; cmd->argv && cmd->argv[i]; i++) {

		char *expanded = expand_word(shell, cmd->argv[i]);

		free(cmd->argv[i]);

		cmd->argv[i] = expanded;
	}

	for (t_list *tmp = cmd->assignments ; tmp; tmp = tmp->next) {

		char *expanded = expand_word(shell, (char *)tmp->content);

		free(tmp->content);

		tmp->content = expanded;
	}

	for (t_list *tmp = cmd->redirs ; tmp; tmp = tmp->next) {

		char *expanded = expand_word(shell, tmp->content ? ((t_redir *)tmp->content)->target : "");

		free(tmp->content ? ((t_redir *)tmp->content)->target : NULL);

		((t_redir *)tmp->content)->target = expanded;
	}

	return 0;
}
