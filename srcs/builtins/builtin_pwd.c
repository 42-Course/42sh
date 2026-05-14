/**
 * @file builtin_pwd.c
 * @brief Implementation of the pwd builtin command for the 42sh shell.
 * @author pulgamecanica
 *
 * Without `-P` we honour POSIX 2.4: print `$PWD` provided it's an
 * absolute path. This is what makes `cd /bin; pwd` show "/bin" instead
 * of the symlink-resolved "/usr/bin" that getcwd(3) would return.
 */

#include "42sh.h"
#include "builtins.h"
#include <string.h>

static int	parse_options(int argc, char **argv, int *physical)
{
	int	i;
	int	j;

	i = 1;
	while (i < argc && argv[i] && argv[i][0] == '-' && argv[i][1])
	{
		if (!ft_strcmp(argv[i], "--"))
			return (i + 1);
		j = 1;
		while (argv[i][j])
		{
			if (argv[i][j] == 'L')
				*physical = 0;
			else if (argv[i][j] == 'P')
				*physical = 1;
			else
			{
				ft_putstr_fd("42sh: pwd: -", 2);
				ft_putchar_fd(argv[i][j], 2);
				ft_putendl_fd(": invalid option", 2);
				return (-1);
			}
			j++;
		}
		i++;
	}
	return (i);
}

static int	print_pwd(const char *path)
{
	ft_putendl_fd(path, 1);
	return (0);
}

int	builtin_pwd(struct s_shell *shell, int argc, char **argv)
{
	int			physical;
	int			i;
	const char	*pwd_var;
	char		*cwd;

	physical = 0;
	i = parse_options(argc, argv, &physical);
	if (i < 0 || i < argc)
	{
		if (i >= 0)
			ft_putendl_fd("42sh: pwd: too many arguments", 2);
		shell->last_exit_status = 1;
		return (1);
	}
	if (!physical)
	{
		pwd_var = var_get_value(shell, "PWD");
		if (pwd_var && pwd_var[0] == '/')
		{
			shell->last_exit_status = print_pwd(pwd_var);
			return (shell->last_exit_status);
		}
	}
	cwd = getcwd(NULL, 0);
	if (!cwd)
	{
		ft_putstr_fd("42sh: pwd: ", 2);
		ft_putendl_fd(strerror(errno), 2);
		shell->last_exit_status = 1;
		return (1);
	}
	shell->last_exit_status = print_pwd(cwd);
	free(cwd);
	return (shell->last_exit_status);
}
