/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXPANDER_H
# define EXPANDER_H

# include "ast.h"
# include "libft.h"
# include <string.h>
# include <stdlib.h>
/*
** Expand a single word to a single string (assignments, redir targets)
** No field splitting, no globbing
*/
char					*expand_word(struct s_shell *shell, const char *word);

/*
** Expand a single word to multiple fields (argv words)
** Includes field splitting and globbing
*/
char					**expand_word_to_fields(struct s_shell *shell,
							const char *word);

/*
** Expand a whole command node (argv, assignments, redirections)
** Called by executor before running each simple command
*/
int						expand_command(struct s_shell *shell, t_cmd *cmd);

#endif
