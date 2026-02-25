/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   job_control.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/24 00:00:00 by pulgamecanica    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef JOB_CONTROL_H
# define JOB_CONTROL_H

# include <sys/types.h>
# include "libft.h"

/*
** Job status
*/
typedef enum e_job_status
{
	JOB_RUNNING,
	JOB_STOPPED,
	JOB_DONE,
	JOB_TERMINATED
}	t_job_status;

/*
** Process in a job.
** Stored in t_job.processes as t_list* (each node->content is a t_process*).
** No *next field — traversal is done via t_list.
*/
typedef struct s_process
{
	pid_t	pid;
	char	*cmd;
	int		status;
	int		completed;
	int		stopped;
}	t_process;

/*
** Job (one per pipeline launched with & or any foreground command).
** Stored in t_shell.jobs as t_list* (each node->content is a t_job*).
** No *next field — traversal is done via t_list.
**
** processes: t_list* of t_process* (all pids in this pipeline)
*/
typedef struct s_job
{
	int				id;
	pid_t			pgid;
	char			*cmd_line;
	t_list			*processes;
	t_job_status	status;
	int				notified;
	int				foreground;
}	t_job;

#endif
