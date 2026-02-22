/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   line_editor.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LINE_EDITOR_H
# define LINE_EDITOR_H

# include <termios.h>
# include <stddef.h>
# include "history.h"

/*
** Line editor state
*/
typedef struct s_line_editor
{
	char				*buffer;
	size_t				buf_size;
	size_t				len;
	size_t				cursor;
	size_t				prompt_len;
	int					term_cols;
	int					term_rows;
	struct termios		orig_termios;
	int					raw_mode;
	t_history			*history;
	char				*saved_line;
	int					in_quote;
	int					line_continuation;
}	t_line_editor;

#endif
