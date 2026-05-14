/**
 * @file builtin_jobs.c
 * @brief `jobs` builtin - list active background jobs.
 * @author pulgamecanica
 */

#include "42sh.h"
#include "builtins.h"
#include "job_control.h"

int	builtin_jobs(t_shell *shell, int argc, char **argv)
{
	t_list	*node;

	(void)argc;
	(void)argv;
	job_update_statuses(shell);
	node = shell->jobs;
	while (node)
	{
		job_print_line(STDOUT_FILENO, shell, LST_JOB(node));
		LST_JOB(node)->notified = 1;
		node = node->next;
	}
	return (0);
}
