/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/22 21:00:00 by wengzhang        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_H
# define LEXER_H

/*
** Token types
*/
typedef enum e_token_type
{
	TOK_WORD,
	TOK_PIPE,
	TOK_AND,
	TOK_OR,
	TOK_SEMICOLON,
	TOK_AMPERSAND,
	TOK_NEWLINE,
	TOK_REDIR_IN,
	TOK_REDIR_OUT,
	TOK_REDIR_APPEND,
	TOK_HEREDOC,
	TOK_REDIR_DUP_IN,
	TOK_REDIR_DUP_OUT,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_EOF,
	TOK_ERROR
}	t_token_type;

/*
** Token node (linked list)
** value: raw token string (quotes preserved for expander)
** io_number: fd number before redirect (-1 if none)
*/
typedef struct s_token
{
	t_token_type		type;
	char				*value;
	int					io_number;
	struct s_token		*next;
}	t_token;

/*
** Lexer functions
*/
t_token					*lexer_tokenize(const char *input);
int						lexer_check_quotes(const char *input,
							char *unclosed_quote);
void					lexer_free_tokens(t_token *tokens);

/*
** Token helpers
*/
t_token					*token_new(t_token_type type, char *value);
void					token_free(t_token *token);

#endif
