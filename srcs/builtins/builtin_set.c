/**
 * @file builtin_set.c
 * @brief Implementation of the set builtin command for the 42sh shell.
 * @author your_login
 *
 * The `set` command manages shell variables. Without arguments, it displays
 * all variables. With `NAME=VALUE` arguments, it sets variables.
 */

#include "42sh.h"
#include "builtins.h"
#include "variables.h"
#include <stdlib.h>

/**
 * @brief Check if a string contains the format "NAME=VALUE"
 * @param str The string to check
 * @param eq_pos Pointer to store the position of '=' (output parameter)
 * @return 1 if valid format, 0 otherwise
 */
static int	is_assignment(const char *str, size_t *eq_pos)
{
	size_t	i;

	if (!str || !eq_pos)
		return (0);
	i = 0;
	while (str[i] && str[i] != '=')
		i++;
	if (i == 0 || str[i] != '=')
		return (0);
	*eq_pos = i;
	return (1);
}

/**
 * @brief Display all variables in the format "NAME=VALUE"
 * @param shell Pointer to the shell state
 * @return 0 on success
 */
static int	print_all_variables(t_shell *shell)
{
	t_list	*node;
	t_var	*var;

	if (!shell)
		return (0);
	node = shell->variables;
	while (node)
	{
		var = LST_VAR(node);
		if (var && var->name)
		{
			ft_putstr_fd(var->name, 1);
			ft_putstr_fd("=", 1);
			if (var->value)
				ft_putstr_fd(var->value, 1);
			ft_putendl_fd("", 1);
		}
		node = node->next;
	}
	return (0);
}

/**
 * @brief Set a variable from an assignment string "NAME=VALUE"
 * @param shell Pointer to the shell state
 * @param assignment The "NAME=VALUE" string
 * @return 0 on success, 1 on error
 */
static int	set_variable_from_assignment(t_shell *shell, const char *assignment)
{
	size_t	eq_pos;
	char	*name;
	char	*value;
	int		result;

	if (!shell || !is_assignment(assignment, &eq_pos))
		return (1);
	name = ft_strsub(assignment, 0, eq_pos);
	if (!name)
		return (1);
	value = ft_strsub(assignment, eq_pos + 1, ft_strlen(assignment) - eq_pos - 1);
	if (!value)
	{
		free(name);
		return (1);
	}
	result = var_set(shell, name, value);
	free(name);
	free(value);
	if (result != 0)
	{
		ft_putendl_fd("42sh: set: invalid variable name", 2);
		return (1);
	}
	return (0);
}

int	builtin_set(struct s_shell *shell, int argc, char **argv)
{
	int	i;
	int	result;

	if (!shell || !argv)
		return (1);
	if (argc == 1)
		return (print_all_variables(shell));
	result = 0;
	i = 1;
	while (i < argc)
	{
		if (set_variable_from_assignment(shell, argv[i]) != 0)
			result = 1;
		i++;
	}
	shell->last_exit_status = result;
	return (result);
}
