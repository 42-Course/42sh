/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   42sh.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SHELL_42SH_H
# define SHELL_42SH_H

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <termios.h>
# include <curses.h>
# include <term.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "libft.h"
# include "variables.h"
# include "history.h"
# include "job_control.h"
# include "line_editor.h"
# include "signals.h"

/*
** Main shell state
*/
typedef struct s_shell
{
	t_var				*variables;
	char				**env;
	int					env_dirty;
	int					last_exit_status;
	int					interactive;
	int					running;
	int					exit_confirmed;
	t_job				*jobs;
	t_job				*current_job;
	pid_t				shell_pgid;
	int					terminal_fd;
	struct termios		original_termios;
	t_history			history;
	t_line_editor		line_editor;
}	t_shell;

/*
** Variable functions (declared here since t_shell is now defined)
*/
char					*var_get_value(t_shell *shell, const char *name);
t_var					*var_get(t_shell *shell, const char *name);
int						var_set(t_shell *shell, const char *name,
							const char *value);
int						var_unset(t_shell *shell, const char *name);
int						var_export(t_shell *shell, const char *name);
char					**var_get_environ(t_shell *shell);
void					var_init_from_environ(t_shell *shell, char **environ);

/*
** Signals check (needs t_shell)
*/
void					signals_check(t_shell *shell);

#endif
