#include "expander.h"

char	**expand_word_to_fields(struct s_shell *shell, const char *word)
{
	if (!shell || word) {
		return (NULL);
	}
	

}
int	expand_command(struct s_shell *shell, t_cmd *cmd)
{
	if (!shell || !cmd ) {
		return (-1);
	}
	char **new_argv = NULL;


	return (0);
}


int						expand_command(struct s_shell *shell, t_cmd *cmd)
{
	if (!shell || !cmd) {
		return (-1);
	}
	return (0);
}
