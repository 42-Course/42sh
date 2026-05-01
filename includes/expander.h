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
# include "42sh.h"
# include "libft.h"
# include <string.h>
# include <stdlib.h>
# include <stdbool.h>
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

/**
 * TODO : the whole function
 * @brief receive a dollar variable input and search in env the said variable
 * 			NOTE : there is two paramater that seems irrelevant to me, I am just
 * 					following the plan for now.
 * @param shell the whole shell structure
 * @param input the variable to expand
 * @param pos the position of the input in the command string (might be irrelevant)
 * @param in_double_quote check if input is in double quote (might be irrelevant)
 * 
 * @return expanded Dollard variable if !NULL else an empty string 
 */
char *expand_dollar(t_shell * shell, char *input, size_t pos, bool in_double_quote);

/**
 * TODO the whole function
 */
char * expand_tilde(t_shell * shell, char * input, size_t pos);

/**
 * TODO the whole function
 */
char *expand_braced_variable(t_shell * shell, char *input, size_t pos, bool in_double_quote);

char *expand_word_internal(t_shell *shell, char *input);

char *expand_arithmetic(t_shell *shell, char *input, size_t pos);

#endif
