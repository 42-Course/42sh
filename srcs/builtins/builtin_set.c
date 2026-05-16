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
 * @brief Display all variables in the format "NAME=VALUE" in alphabetical order.
 * @param shell Pointer to the shell state
 * @return 0 on success
 */
static int	cmp_vars(const void *a, const void *b)
{
	t_var *va = *(t_var **)a;
	t_var *vb = *(t_var **)b;

	if (!va && !vb)
		return (0);
	if (!va)
		return (-1);
	if (!vb)
		return (1);
	if (!va->name && !vb->name)
		return (0);
	if (!va->name)
		return (1);
	if (!vb->name)
		return (-1);
	return (ft_strcmp(va->name, vb->name));
}

static int	print_all_variables(t_shell *shell)
{
	t_list	*node;
	t_var	**arr;
	size_t	count;
	size_t	i;

	if (!shell)
		return (0);
	count = 0;
	node = shell->variables;
	while (node)
	{
		if (LST_VAR(node) && LST_VAR(node)->name)
			count++;
		node = node->next;
	}
	if (count == 0)
		return (0);
	arr = malloc(sizeof(*arr) * count);
	if (!arr)
		return (0);
	node = shell->variables;
	i = 0;
	while (node)
	{
		if (LST_VAR(node) && LST_VAR(node)->name)
			arr[i++] = LST_VAR(node);
		node = node->next;
	}
	qsort(arr, count, sizeof(*arr), cmp_vars);
	i = 0;
	while (i < count)
	{
		if (arr[i]->name)
			ft_putstr_fd(arr[i]->name, 1);
		ft_putstr_fd("=", 1);
		if (arr[i]->value)
			ft_putstr_fd(arr[i]->value, 1);
		ft_putendl_fd("", 1);
		i++;
	}
	free(arr);
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
