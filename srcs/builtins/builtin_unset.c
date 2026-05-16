/**
 * @file builtin_unset.c
 * @brief Implementation of the unset builtin command for the 42sh shell.
 * @author your_login
 *
 * The `unset` command removes shell variables by name.
 */

#include "42sh.h"
#include "builtins.h"
#include "variables.h"
#include <stdlib.h>

/**
 * @brief The unset builtin command.
 *
 * **Usage**: `unset name [name ...]`
 *
 * Removes one or more variables by name. Each argument should be a valid
 * variable name (identifier). If a name is not found, unset succeeds silently
 * (POSIX behavior).
 *
 * **Examples**:
 * @code{.sh}
 * unset var1           # remove a variable
 * unset x y z          # remove multiple variables
 * unset nonexistent    # succeeds silently
 * @endcode
 *
 * @param shell Pointer to the shell state
 * @param argc Argument count
 * @param argv Argument vector
 * @return 0 on success, 1 on error (invalid identifier)
 */
int	builtin_unset(struct s_shell *shell, int argc, char **argv)
{
	int	i;
	int	result;

	if (!shell || !argv)
		return (1);
	if (argc < 2)
	{
		ft_putendl_fd("42sh: unset: missing operand", 2);
		shell->last_exit_status = 1;
		return (1);
	}
	result = 0;
	i = 1;
	while (i < argc)
	{
		if (!var_is_valid_identifier(argv[i]))
		{
			ft_putstr_fd("42sh: unset: `", 2);
			ft_putstr_fd(argv[i], 2);
			ft_putendl_fd("': not a valid identifier", 2);
			result = 1;
		}
		else
		{
			var_unset(shell, argv[i]);
		}
		i++;
	}
	shell->last_exit_status = result;
	return (result);
}
