/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALS_H
# define SIGNALS_H

# include <signal.h>

/*
** Global signal flag (volatile for signal handler safety)
*/
extern volatile sig_atomic_t	g_signal_received;

/*
** Signal context setup functions
** P4 implements: signals_setup_interactive
** P2 implements: signals_setup_executing, signals_setup_child
*/
void							signals_setup_interactive(void);
void							signals_setup_executing(void);
void							signals_setup_child(void);

#endif
