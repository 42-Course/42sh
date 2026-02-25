/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   line_editor.h                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pulgamecanica <pulgamecanica@student.42.fr> +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/24 00:00:00 by pulgamecanica    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LINE_EDITOR_H
# define LINE_EDITOR_H

# include <stddef.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "history.h"

/*
** Line editor state.
**
** We use GNU readline for input: it handles raw mode, key reading, buffer
** management, cursor movement, and display refresh internally.
** This struct holds our configuration and integration state on top of it.
**
** history:      pointer to t_history (owned by t_shell) — our custom history
**               module used alongside readline for file persistence and history
**               expansions.  We call add_history() (readline) AND history_add()
**               (ours) after each accepted line.
**
** saved_line:   buffer saved before history navigation so we can restore it
**               when the user presses Down past the newest entry.
**               (Only relevant if we override readline's history callbacks.)
**
** editing_mode: 0 = readline/emacs (default), 1 = vi.
**               Changed at runtime via `set -o vi` / `set -o emacs`.
**               Applied with rl_variable_bind("editing-mode", ...).
*/
typedef struct s_line_editor
{
	t_history	*history;
	char		*saved_line;
	int			editing_mode;
}	t_line_editor;

/*
** Editing mode values for s_line_editor.editing_mode
*/
# define LE_MODE_EMACS	0
# define LE_MODE_VI		1

/*
** Interface
**
** line_editor_init:     configure readline (history file, bindings, signals).
** line_editor_cleanup:  save history, free saved_line.
** line_editor_readline: prompt and read one line; returns malloc'd string or
**                       NULL on EOF.  Handles history integration, Ctrl-C,
**                       and multi-line continuation (unclosed quotes / '\').
** line_editor_set_mode: switch between emacs and vi editing mode at runtime.
*/
int		line_editor_init(t_line_editor *le, t_history *history);
void	line_editor_cleanup(t_line_editor *le);
char	*line_editor_readline(t_line_editor *le, const char *prompt);
void	line_editor_set_mode(t_line_editor *le, int mode);

#endif
