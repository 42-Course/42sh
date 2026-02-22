/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42sh.h"
#include "ft_printf.h"
#include <stdio.h>

static int	shell_init(t_shell *shell, char **envp)
{
	ft_bzero(shell, sizeof(t_shell));
	shell->interactive = isatty(STDIN_FILENO);
	shell->running = 1;
	shell->terminal_fd = STDIN_FILENO;
	shell->shell_pgid = getpid();
	shell->env_dirty = 1;
	(void)envp;
	return (0);
}

static void	shell_cleanup(t_shell *shell)
{
	(void)shell;
}

static char	*read_line(t_shell *shell)
{
	char	*line;
	size_t	len;
	ssize_t	nread;

	(void)shell;
	line = NULL;
	len = 0;
	nread = getline(&line, &len, stdin);
	if (nread <= 0)
	{
		free(line);
		return (NULL);
	}
	if (nread > 0 && line[nread - 1] == '\n')
		line[nread - 1] = '\0';
	return (line);
}

int	main(int argc, char **argv, char **envp)
{
	t_shell	shell;
	char	*line;

	(void)argc;
	(void)argv;
	shell_init(&shell, envp);
	while (shell.running)
	{
		if (shell.interactive)
			ft_dprintf(STDERR_FILENO, "42sh$ ");
		line = read_line(&shell);
		if (!line)
		{
			if (shell.interactive)
				ft_dprintf(STDERR_FILENO, "exit\n");
			break ;
		}
		if (*line == '\0')
		{
			free(line);
			continue ;
		}
		free(line);
	}
	shell_cleanup(&shell);
	return (shell.last_exit_status);
}
