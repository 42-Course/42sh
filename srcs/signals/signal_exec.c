/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal_exec.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 00:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/04/06 00:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "signals.h"
#include <stddef.h>

/*
** Set up signal handlers for executing context (parent waiting for fg child).
**
** SIGINT  — ignored (let signal reach the child process)
** SIGQUIT — ignored (let signal reach the child process)
** SIGTSTP — ignored (let signal reach the child process)
**
** The parent simply waits; the child handles or dies from the signal.
*/
void	signals_setup_executing(void)
{
	struct sigaction	sa;

	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTSTP, &sa, NULL);
}
