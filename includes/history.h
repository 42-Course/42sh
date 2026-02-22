/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   history.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HISTORY_H
# define HISTORY_H

# define HISTORY_MAX_SIZE	500

/*
** History entry (doubly linked list)
*/
typedef struct s_history_entry
{
	int						number;
	char					*line;
	struct s_history_entry	*prev;
	struct s_history_entry	*next;
}	t_history_entry;

/*
** History state
*/
typedef struct s_history
{
	t_history_entry		*head;
	t_history_entry		*tail;
	t_history_entry		*current;
	int					count;
	int					max_size;
	int					next_number;
	char				*file_path;
}	t_history;

/*
** History functions
*/
void					history_init(t_history *hist, int max_size);
void					history_free(t_history *hist);
int						history_add(t_history *hist, const char *line);
char					*history_prev(t_history *hist);
char					*history_next(t_history *hist);
void					history_reset_cursor(t_history *hist);
int						history_load(t_history *hist, const char *path);
int						history_save(t_history *hist, const char *path);

#endif
