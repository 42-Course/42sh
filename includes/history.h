/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   history.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/24 00:00:00 by pulgamecanica    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HISTORY_H
# define HISTORY_H

# include "libft.h"

# define HISTORY_MAX_SIZE	500

/*
** History entry data.
** Stored in t_history as t_dlist* nodes (each node->content is a
** t_history_entry*). The doubly-linked list allows O(1) prev/next navigation
** for up/down arrow keys without extra cursor tracking complexity.
*/
typedef struct s_history_entry
{
	int		number;
	char	*line;
}	t_history_entry;

/*
** History state.
** head → oldest entry  (t_dlist*, content = t_history_entry*)
** tail → newest entry  (t_dlist*, content = t_history_entry*)
** current → navigation cursor (NULL when not navigating)
*/
typedef struct s_history
{
	t_dlist	*head;
	t_dlist	*tail;
	t_dlist	*current;
	int		count;
	int		max_size;
	int		next_number;
	char	*file_path;
}	t_history;

/*
** Lifecycle
*/
void	history_init(t_history *hist, int max_size);
void	history_free(t_history *hist);

/*
** Add a command to history (call after each accepted line).
** Skips empty/whitespace-only lines and consecutive duplicates.
*/
int		history_add(t_history *hist, const char *line);

/*
** Navigation — called by line editor on up/down arrow.
** history_prev: returns older line or NULL if already at oldest.
** history_next: returns newer line or NULL if past newest (restore saved_line).
*/
char	*history_prev(t_history *hist);
char	*history_next(t_history *hist);

/*
** Reset navigation cursor to NULL (call before each new readline session).
*/
void	history_reset_cursor(t_history *hist);

/*
** File persistence: load at startup, save at exit.
** Path may contain ~ (expanded internally).
** File format: one command per line, plain text, most recent last.
** File permissions: 0600.
*/
int		history_load(t_history *hist, const char *path);
int		history_save(t_history *hist, const char *path);

#endif
