/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   job_control.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef JOB_CONTROL_H
# define JOB_CONTROL_H

# include <sys/types.h>

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
** Process in a job (linked list)
*/
typedef struct s_process
{
	pid_t				pid;
	char				*cmd;
	int					status;
	int					completed;
	int					stopped;
	struct s_process	*next;
}	t_process;

/*
** Job (linked list)
*/
typedef struct s_job
{
	int					id;
	pid_t				pgid;
	char				*cmd_line;
	t_process			*processes;
	t_job_status		status;
	int					notified;
	int					foreground;
	struct s_job		*next;
}	t_job;

#endif
