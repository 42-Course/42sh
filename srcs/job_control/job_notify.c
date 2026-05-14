/**
 * @file job_notify.c
 * @brief Reap background jobs and print status notifications.
 * @author pulgamecanica
 */

#include "42sh.h"
#include "job_control.h"
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

#define JOB_STATUS_COL	22

const char	*job_status_str(t_job_status s)
{
	if (s == JOB_RUNNING)
		return ("Running");
	if (s == JOB_STOPPED)
		return ("Stopped");
	if (s == JOB_DONE)
		return ("Done");
	return ("Terminated");
}

void	job_recompute_status(t_job *job)
{
	t_list		*node;
	t_process	*p;
	int			all_done;
	int			any_stopped;

	all_done = 1;
	any_stopped = 0;
	node = job->processes;
	while (node)
	{
		p = LST_PROC(node);
		if (!p->completed)
			all_done = 0;
		if (p->stopped)
			any_stopped = 1;
		node = node->next;
	}
	if (all_done)
	{
		if (job->status != JOB_TERMINATED)
			job->status = JOB_DONE;
	}
	else if (any_stopped)
		job->status = JOB_STOPPED;
	else
		job->status = JOB_RUNNING;
}

int	job_apply_status(t_job *job, pid_t pid, int status)
{
	t_list		*node;
	t_process	*p;

	node = job->processes;
	while (node)
	{
		p = LST_PROC(node);
		if (p->pid != pid)
		{
			node = node->next;
			continue ;
		}
		p->status = status;
		if (WIFSTOPPED(status))
			p->stopped = 1;
		else
		{
			p->completed = 1;
			p->stopped = 0;
			if (WIFSIGNALED(status))
				job->status = JOB_TERMINATED;
		}
		job->notified = 0;
		return (1);
	}
	return (0);
}

static void	apply_wait_result(t_shell *shell, pid_t pid, int status)
{
	t_job	*job;

	job = job_find_by_pid(shell, pid);
	if (!job)
		return ;
	if (job_apply_status(job, pid, status))
		job_recompute_status(job);
}

void	job_update_statuses(t_shell *shell)
{
	pid_t	pid;
	int		status;

	while (1)
	{
		pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);
		if (pid <= 0)
			break ;
		apply_wait_result(shell, pid, status);
	}
}

/**
 * @brief Map a signal number to its short POSIX name (no "SIG" prefix
 *        stripped — we want "SIGTTOU" to match bash's listing format).
 * @return Pointer to static string, or NULL if unknown.
 */
static const char	*sig_short_name(int sig)
{
	if (sig == SIGHUP)	return ("SIGHUP");
	if (sig == SIGINT)	return ("SIGINT");
	if (sig == SIGQUIT)	return ("SIGQUIT");
	if (sig == SIGILL)	return ("SIGILL");
	if (sig == SIGABRT)	return ("SIGABRT");
	if (sig == SIGFPE)	return ("SIGFPE");
	if (sig == SIGKILL)	return ("SIGKILL");
	if (sig == SIGSEGV)	return ("SIGSEGV");
	if (sig == SIGPIPE)	return ("SIGPIPE");
	if (sig == SIGALRM)	return ("SIGALRM");
	if (sig == SIGTERM)	return ("SIGTERM");
	if (sig == SIGUSR1)	return ("SIGUSR1");
	if (sig == SIGUSR2)	return ("SIGUSR2");
	if (sig == SIGSTOP)	return ("SIGSTOP");
	if (sig == SIGTSTP)	return ("SIGTSTP");
	if (sig == SIGTTIN)	return ("SIGTTIN");
	if (sig == SIGTTOU)	return ("SIGTTOU");
	if (sig == SIGBUS)	return ("SIGBUS");
	return (NULL);
}

/**
 * @brief Find the stop signal of the first stopped process in the job.
 * @return Signal number, or 0 if no process is stopped.
 */
static int	job_stop_signal(t_job *job)
{
	t_list		*node;
	t_process	*p;

	node = job->processes;
	while (node)
	{
		p = LST_PROC(node);
		if (p->stopped && WIFSTOPPED(p->status))
			return (WSTOPSIG(p->status));
		node = node->next;
	}
	return (0);
}

/**
 * @brief Append "(SIGFOO)" or "(N)" to a status string when relevant.
 * @details Bash only shows the signal suffix for stops that are NOT
 *          SIGTSTP (Ctrl-Z is implicit). Falls back to the integer if
 *          the signal isn't in our name table.
 * @return Total bytes written to @p buf (excluding NUL).
 */
static size_t	put_status(t_job *job, char *buf, size_t size)
{
	const char	*base;
	const char	*name;
	int			sig;

	base = job_status_str(job->status);
	sig = 0;
	if (job->status == JOB_STOPPED)
		sig = job_stop_signal(job);
	if (sig > 0 && sig != SIGTSTP)
	{
		name = sig_short_name(sig);
		if (name)
			return ((size_t)snprintf(buf, size, "%s(%s)", base, name));
		return ((size_t)snprintf(buf, size, "%s(%d)", base, sig));
	}
	return ((size_t)snprintf(buf, size, "%s", base));
}

/**
 * @brief Pick the current-job marker: '+' for current, ' ' otherwise.
 * @details Bash also has '-' for the previous current job; we don't
 *          track previous yet, so it stays a space.
 */
static char	job_marker(t_shell *shell, t_job *job)
{
	if (shell && shell->current_job == job)
		return ('+');
	return (' ');
}

/**
 * @brief Map a fatal signal to bash's foreground death message.
 * @return Description string, or NULL when the signal should be silent
 *         (SIGINT, SIGPIPE) or isn't in our table.
 */
static const char	*sig_termination_desc(int sig)
{
	if (sig == SIGHUP)	return ("Hangup");
	if (sig == SIGINT)	return (NULL);
	if (sig == SIGQUIT)	return ("Quit");
	if (sig == SIGILL)	return ("Illegal instruction");
	if (sig == SIGTRAP)	return ("Trace/breakpoint trap");
	if (sig == SIGABRT)	return ("Aborted");
	if (sig == SIGBUS)	return ("Bus error");
	if (sig == SIGFPE)	return ("Floating point exception");
	if (sig == SIGKILL)	return ("Killed");
	if (sig == SIGUSR1)	return ("User defined signal 1");
	if (sig == SIGSEGV)	return ("Segmentation fault");
	if (sig == SIGUSR2)	return ("User defined signal 2");
	if (sig == SIGPIPE)	return (NULL);
	if (sig == SIGALRM)	return ("Alarm clock");
	if (sig == SIGTERM)	return ("Terminated");
	if (sig == SIGXCPU)	return ("CPU time limit exceeded");
	if (sig == SIGXFSZ)	return ("File size limit exceeded");
	if (sig == SIGSYS)	return ("Bad system call");
	return (NULL);
}

/**
 * @brief Return the wstatus of the last process in a job's pipeline.
 * @details Mirrors last_process_exit() in job_wait.c — that's the
 *          process whose exit status the user observes as $?.
 */
static int	last_process_status(t_job *job)
{
	t_list		*node;
	t_process	*last;

	last = NULL;
	node = job->processes;
	while (node)
	{
		last = LST_PROC(node);
		node = node->next;
	}
	if (!last)
		return (0);
	return (last->status);
}

void	job_print_termination(t_job *job)
{
	int			status;
	int			sig;
	const char	*desc;

	if (!job || job->status != JOB_TERMINATED)
		return ;
	status = last_process_status(job);
	if (!WIFSIGNALED(status))
		return ;
	sig = WTERMSIG(status);
	desc = sig_termination_desc(sig);
	if (!desc)
		return ;
	ft_putstr_fd((char *)desc, STDERR_FILENO);
#ifdef WCOREDUMP
	if (WCOREDUMP(status))
		ft_putstr_fd(" (core dumped)", STDERR_FILENO);
#endif
	ft_putendl_fd("", STDERR_FILENO);
}

void	job_print_line(int fd, t_shell *shell, t_job *job)
{
	char	status[64];
	size_t	len;
	size_t	pad;

	ft_putstr_fd("[", fd);
	ft_putnbr_fd(job->id, fd);
	ft_putstr_fd("]", fd);
	ft_putchar_fd(job_marker(shell, job), fd);
	ft_putstr_fd("  ", fd);
	len = put_status(job, status, sizeof(status));
	ft_putstr_fd(status, fd);
	pad = (len < JOB_STATUS_COL) ? JOB_STATUS_COL - len : 0;
	while (pad-- > 0)
		ft_putchar_fd(' ', fd);
	ft_putstr_fd("  ", fd);
	ft_putstr_fd(job->cmd_line ? job->cmd_line : "", fd);
	ft_putstr_fd("\n", fd);
}

static void	lst_remove_node(t_list **head, t_list *target,
	void (*del)(void *))
{
	t_list	*prev;
	t_list	*node;

	if (!head || !*head || !target)
		return ;
	if (*head == target)
	{
		*head = target->next;
		ft_lstdelone(&target, del);
		return ;
	}
	prev = *head;
	node = (*head)->next;
	while (node)
	{
		if (node == target)
		{
			prev->next = node->next;
			ft_lstdelone(&node, del);
			return ;
		}
		prev = node;
		node = node->next;
	}
}

void	job_notify(t_shell *shell)
{
	t_list	*node;
	t_list	*next;
	t_job	*job;

	node = shell->jobs;
	while (node)
	{
		next = node->next;
		job = LST_JOB(node);
		if (!job->notified && job->status != JOB_RUNNING)
		{
			job_print_line(STDERR_FILENO, shell, job);
			job->notified = 1;
		}
		if (job->status == JOB_DONE || job->status == JOB_TERMINATED)
		{
			if (shell->current_job == job)
				shell->current_job = NULL;
			lst_remove_node(&shell->jobs, node, job_free);
		}
		node = next;
	}
}

void	job_control_cleanup(t_shell *shell)
{
	ft_lstdel(&shell->jobs, job_free);
	shell->jobs = NULL;
	shell->current_job = NULL;
}
