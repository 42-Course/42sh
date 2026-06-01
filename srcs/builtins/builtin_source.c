/**
 * @file builtin_source.c
 * @brief The `source` / `.` builtin.
 * @author pulgamecanica
 */

#include "42sh.h"
#include "builtins.h"
#include <string.h>

int	builtin_source(t_shell *shell, int argc, char **argv)
{
	FILE	*f;

	if (argc < 2)
	{
		ft_putendl_fd("42sh: source: filename argument required", 2);
		return (2);
	}
	f = fopen(argv[1], "r");
	if (!f)
	{
		ft_putstr_fd("42sh: source: ", 2);
		ft_putstr_fd(argv[1], 2);
		ft_putstr_fd(": ", 2);
		ft_putendl_fd(strerror(errno), 2);
		return (1);
	}
	shell_run_stream(shell, f);
	fclose(f);
	return (shell->last_exit_status);
}
