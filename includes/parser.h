/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_H
# define PARSER_H

# include "lexer.h"
# include "ast.h"

/*
** Parser state
*/
typedef struct s_parser
{
	t_token				*tokens;
	t_token				*current;
	char				*error;
}	t_parser;

/*
** Parser functions
*/
t_ast					*parser_parse(t_token *tokens);
int						parser_collect_heredocs(struct s_shell *shell,
							t_ast *ast);

#endif
