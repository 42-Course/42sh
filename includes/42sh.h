/**
 * @file 42sh.h
 * @brief Top-level header for the 42sh shell, includes all sub-systems and
 *        defines the central `t_shell` state struct.
 * @author wengzhang, jguillem, jspitz, pulgamecanica, zweng
 */

#ifndef SHELL_42SH_H
# define SHELL_42SH_H

# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>
# include <errno.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <termios.h>
# include <term.h>
# include "libft.h"
# include "variables.h"
# include "history.h"
# include "job_control.h"
# include "line_editor.h"
# include "signals.h"

/** 
 * @brief Central runtime state of the shell — one instance lives in `main()`.
 * @details This struct holds all global state, including variables, jobs, history, and
 *          line editor.
 */
typedef struct s_shell
{
	t_list		*aliases;         /**< Linked list using `t_alias*` as content; alias table. */
	t_job			*current_job;     /**< Most recent job (`%+` / `%%`). */
	char			**env;            /**< Cached NULL-terminated array for execve (lazy rebuild). */
	int				env_dirty;        /**< 1 when `env` needs rebuild before next execve. */
	int				exit_confirmed;   /**< Double-exit confirmation when stopped jobs exist. */
	t_history	history;          /**< Embedded history state (pointer passed to line editor). */
	int				interactive;      /**< 1 if stdin is a TTY (prompt + line editor active). */
	t_list		*jobs;            /**< `t_job*` list; all known jobs. */
	int				last_exit_status; /**< Value of `$?` (exit status of last foreground command). */
	t_line_editor		line_editor;      /**< Embedded line editor state. */
	struct termios	original_termios; /**< Saved terminal attributes, restored on exit. */
	int				running;          /**< Main loop flag; set to 0 to exit. */
	pid_t			shell_pgid;       /**< Shell's own process group id (set at startup). */
	int				terminal_fd;      /**< File descriptor of the controlling terminal. */
	t_list		*variables;       /**< `t_var*` list; all shell/env variables. */
}	t_shell;

/** Get the value string of variable `name`, or NULL if unset. */
char	*var_get_value(t_shell *shell, const char *name);

/** Get the `t_var` node for `name`, or NULL if unset. */
t_var	*var_get(t_shell *shell, const char *name);

/** Set (or create) variable `name` to `value`. Returns 0 on success. */
int		var_set(t_shell *shell, const char *name, const char *value);

/** Remove variable `name`. Returns 0 on success. */
int		var_unset(t_shell *shell, const char *name);

/** Mark variable `name` for export to child processes. */
int		var_export(t_shell *shell, const char *name);

/** Return (and cache) the `NULL`-terminated `envp` array for execve. */
char	**var_get_environ(t_shell *shell);

/** Populate `shell->variables` from the process's initial `envp`. */
void	var_init_from_environ(t_shell *shell, char **envp);

/** Process any pending signals; called in the main loop before each prompt. */
void	signals_check(t_shell *shell);

/** Cast a `t_list` node's content to `t_var *`. */
# define LST_VAR(n)		((t_var *)(n)->content)

/** Cast a `t_list` node's content to `t_job *`. */
# define LST_JOB(n)		((t_job *)(n)->content)

/** Cast a `t_list` node's content to `t_alias *`. */
# define LST_ALIAS(n)	((t_alias *)(n)->content)

/** Cast a `t_list` node's content to `t_process *`. */
# define LST_PROC(n)	((t_process *)(n)->content)

#endif
