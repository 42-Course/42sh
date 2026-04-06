/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal_child.c                                   :+:      :+:    :+:   */
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
** Restore default signal handlers in a child process before execve.
**
** After fork the child inherits the parent's signal dispositions (SIG_IGN
** for interactive mode).  We must reset them to SIG_DFL so the executed
** program responds to signals normally.
*/
void	signals_setup_child(void)
{
	struct sigaction	sa;

	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_DFL;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTSTP, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
}
