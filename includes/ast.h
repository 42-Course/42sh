/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ast.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef AST_H
# define AST_H

# include "lexer.h"

/*
** AST node types
*/
typedef enum e_node_type
{
	NODE_COMMAND,
	NODE_PIPE,
	NODE_AND,
	NODE_OR,
	NODE_SEQUENCE,
	NODE_SUBSHELL,
	NODE_BLOCK,
	NODE_BACKGROUND
}	t_node_type;

/*
** Redirection node
*/
typedef struct s_redir
{
	t_token_type		type;
	int					fd;
	char				*target;
	char				*heredoc_delim;
	char				*heredoc_content;
	int					heredoc_quoted;
	struct s_redir		*next;
}	t_redir;

/*
** Simple command data
*/
typedef struct s_cmd
{
	char				**argv;
	int					argc;
	char				**assignments;
	t_redir				*redirs;
}	t_cmd;

/*
** Binary operation data (pipe, &&, ||, ;)
*/
typedef struct s_binary
{
	struct s_ast		*left;
	struct s_ast		*right;
}	t_binary;

/*
** Group data (subshell, block, background)
*/
typedef struct s_group
{
	struct s_ast		*child;
	t_redir				*redirs;
}	t_group;

/*
** AST node - union-based
*/
typedef struct s_ast
{
	t_node_type			type;
	union
	{
		t_cmd			cmd;
		t_binary		binary;
		t_group			group;
	}	data;
}	t_ast;

/*
** AST functions
*/
t_ast					*ast_new_command(t_cmd *cmd);
t_ast					*ast_new_binary(t_node_type type, t_ast *left,
							t_ast *right);
t_ast					*ast_new_group(t_node_type type, t_ast *child,
							t_redir *redirs);
void					ast_free(t_ast *node);

/*
** Redirection functions
*/
t_redir					*redir_new(t_token_type type, int fd, char *target);
void					redir_free(t_redir *redir);
void					redir_list_free(t_redir *head);

#endif
