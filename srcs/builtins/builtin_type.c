/**
 * @file builtin_type.c
 * @brief Implementation of the type builtin command for the 42sh shell.
 * @author zweng
 */

#include "42sh.h"
#include "builtins.h"
#include "executor.h"
#include "variables.h"

#define TYPE_F_T 1
#define TYPE_F_P 2
#define TYPE_F_A 4

/**
 * @brief Report an unrecognised option letter on stderr.
 */
static void	type_invalid_option(char opt)
{
	ft_putstr_fd("42sh: type: -", 2);
	write(2, &opt, 1);
	ft_putendl_fd(": invalid option", 2);
}

/**
 * @brief Parse leading -t/-p/-a options (combinable; `--` ends parsing).
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param flags Out: bitmask of TYPE_F_* for the options seen.
 * @return Index of the first name argument, or -1 on an invalid option.
 */
static int	parse_type_opts(int argc, char **argv, int *flags)
{
	int		i;
	char	*p;

	*flags = 0;
	i = 0;
	while (++i < argc && argv[i][0] == '-' && argv[i][1] != '\0')
	{
		if (argv[i][1] == '-' && argv[i][2] == '\0')
			return (i + 1);
		p = argv[i];
		while (*++p)
		{
			if (*p == 't')
				*flags |= TYPE_F_T;
			else if (*p == 'p')
				*flags |= TYPE_F_P;
			else if (*p == 'a')
				*flags |= TYPE_F_A;
			else
				return (type_invalid_option(*p), -1);
		}
	}
	return (i);
}

/**
 * @brief Free a NULL-terminated string array.
 */
static void	type_free_dirs(char **dirs)
{
	int	i;

	i = 0;
	while (dirs[i])
		free(dirs[i++]);
	free(dirs);
}

/**
 * @brief Build "dir/name". Caller frees the result.
 */
static char	*type_join(const char *dir, const char *name)
{
	char	*tmp;
	char	*full;

	tmp = ft_strjoin(dir, "/");
	if (!tmp)
		return (NULL);
	full = ft_strjoin(tmp, name);
	free(tmp);
	return (full);
}

/**
 * @brief Print every location holding an executable `name`.
 * @details A name containing '/' is probed directly; otherwise every
 *          $PATH directory is walked and each match printed.
 * @return 1 if at least one match was printed, 0 otherwise.
 */
static int	type_path_matches(t_shell *shell, const char *name)
{
	char	**dirs;
	char	*full;
	int		found;
	int		i;

	if (ft_strchr(name, '/'))
	{
		if (access(name, X_OK) != 0)
			return (0);
		return (printf("%s is %s\n", name, name), 1);
	}
	if (!var_get_value(shell, "PATH"))
		return (0);
	dirs = ft_strsplit(var_get_value(shell, "PATH"), ':');
	if (!dirs)
		return (0);
	found = 0;
	i = -1;
	while (dirs[++i])
	{
		full = type_join(dirs[i], name);
		if (full && access(full, X_OK) == 0 && ++found)
			printf("%s is %s\n", name, full);
		free(full);
	}
	return (type_free_dirs(dirs), found != 0);
}

/**
 * @brief `type -t`: print the one-word category (builtin/file).
 */
static int	type_kind(t_shell *shell, const char *name)
{
	char	*path;

	if (builtin_is_builtin(name))
		return (printf("builtin\n"), 0);
	path = find_command(shell, name);
	if (!path)
		return (1);
	free(path);
	return (printf("file\n"), 0);
}

/**
 * @brief `type -p`: print the resolved path of a disk command only.
 */
static int	type_path(t_shell *shell, const char *name)
{
	char	*path;

	if (builtin_is_builtin(name))
		return (0);
	path = find_command(shell, name);
	if (!path)
		return (1);
	printf("%s\n", path);
	free(path);
	return (0);
}

/**
 * @brief `type -a`: print the builtin entry and every PATH match.
 */
static int	type_all(t_shell *shell, const char *name)
{
	int	found;

	found = 0;
	if (builtin_is_builtin(name))
	{
		printf("%s is a shell builtin\n", name);
		found = 1;
	}
	if (type_path_matches(shell, name))
		found = 1;
	if (found)
		return (0);
	ft_putstr_fd(name, 2);
	ft_putendl_fd(" not found", 2);
	return (1);
}

/**
 * @brief Default form: "<name> is a shell builtin" / "<name> is <path>".
 */
static int	type_default(t_shell *shell, const char *name)
{
	char	*path;

	if (builtin_is_builtin(name))
		return (printf("%s is a shell builtin\n", name), 0);
	path = find_command(shell, name);
	if (path)
	{
		printf("%s is %s\n", name, path);
		free(path);
		return (0);
	}
	ft_putstr_fd(name, 2);
	ft_putendl_fd(" not found", 2);
	return (1);
}

/**
 * @brief Route one name to the handler selected by the parsed flags.
 */
static int	type_dispatch(t_shell *shell, const char *name, int flags)
{
	if (flags & TYPE_F_T)
		return (type_kind(shell, name));
	if (flags & TYPE_F_P)
		return (type_path(shell, name));
	if (flags & TYPE_F_A)
		return (type_all(shell, name));
	return (type_default(shell, name));
}

int	builtin_type(struct s_shell *shell, int argc, char **argv)
{
	int	flags;
	int	first;
	int	status;
	int	i;

	first = parse_type_opts(argc, argv, &flags);
	if (first < 0)
	{
		shell->last_exit_status = 2;
		return (2);
	}
	status = 0;
	i = first - 1;
	while (++i < argc)
		status |= type_dispatch(shell, argv[i], flags);
	shell->last_exit_status = status;
	return (status);
}
