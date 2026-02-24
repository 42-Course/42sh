/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/24 00:00:00 by pulgamecanica    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_H
# define LEXER_H

# include "libft.h"

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
** Token data node.
** Stored in a t_list* returned by lexer_tokenize (each node->content is a
** t_token*). No *next field — traversal is via the t_list wrapper.
**
** value:     raw token string (quotes preserved for expander).
** io_number: fd number before a redirect operator (-1 if none).
*/
typedef struct s_token
{
	t_token_type	type;
	char			*value;
	int				io_number;
}	t_token;

/*
** Lexer interface
**
** lexer_tokenize: tokenize input string.
**   Returns a t_list* (nodes contain t_token* via node->content).
**   Returns NULL on error (error already printed to stderr).
**   Caller must free with lexer_free_tokens().
**
** lexer_check_quotes: check for unclosed quotes.
**   Sets *unclosed_quote to the quote char ('\'', '"') or 0 if balanced.
**   Returns 1 if open quote found, 0 if balanced.
**
** lexer_free_tokens: free entire token list (tokens + strings + nodes).
*/
t_list				*lexer_tokenize(const char *input);
int					lexer_check_quotes(const char *input, char *unclosed_quote);
void				lexer_free_tokens(t_list *tokens);

/*
** Token helpers (used by lexer internally and by tests)
*/
t_token				*token_new(t_token_type type, char *value);
void				token_free(t_token *token);

/*
** Convenience accessor: get t_token* from a t_list node.
*/
# define TOK(node)	((t_token *)(node)->content)

#endif
