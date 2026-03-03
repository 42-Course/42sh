/**
 * @file lexer_operator.c
 * @brief file to manage operators of the prompt command
 * @author jguillem
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

/**
 * @param c character to check
 * @brief check if a char is the beginning of an operator
 * @details check if the character is in "&|><;{}\n"
 * @return 0 | 1
 */
int	is_operator(char c)
{
	return (c == '&' || c == '|' || c == '>' || c == '<'
		|| c == ';' || c == '(' || c == ')' || c == '\n');
}

/**
 * @param line string of operator to test
 * @brief check if a string is an operator
 * @details manage the digits in case of redirection then call is_operator
 * @return 1 | 0
 */
int	is_operator_start(const char *line)
{
	if (isdigit(*line))
	{
		while (*line && isdigit(*line))
			line++;
		if (*line == '<' || *line == '>')
			return (1);
		return (0);
	}
	return (is_operator(*line));
}

/**
 * @param line the string of operator
 * @param io_number the io number to redirect
 * @brief create a new t_list *node with a t_token *content
 * @details create a TOK_SEMICOLON
 * @return t_list *
 */
static t_list	*tok_semicolon(const char **line, int io_number)
{
	t_list	*tok;

	tok = token_new(TOK_SEMICOLON, strdup(";"), io_number);
	(*line)++;
	return (tok);
}

/**
 * @param line the string of operator
 * @param io_number the io number to redirect
 * @brief create a new t_list *node with a t_token *content
 * @details create a TOK_NEWLINE
 * @return t_list *
 */
static t_list	*tok_newline(const char **line, int io_number)
{
	t_list	*tok;

	tok = token_new(TOK_NEWLINE, strdup("\n"), io_number);
	(*line)++;
	return (tok);
}

/**
 * @param line the string of operator
 * @param io_number the io number to redirect
 * @brief create a new t_list *node with a t_token *content
 * @details create a TOK_LPAREN
 * @return t_list *
 */
static t_list	*tok_lparen(const char **line, int io_number)
{
	t_list	*tok;

	tok = token_new(TOK_LPAREN, strdup("("), io_number);
	(*line)++;
	return (tok);
}

/**
 * @param line the string of operator
 * @param io_number the io number to redirect
 * @brief create a new t_list *node with a t_token *content
 * @details create a TOK_RPAREN
 * @return t_list *
 */
static t_list	*tok_rparen(const char **line, int io_number)
{
	t_list	*tok;

	tok = token_new(TOK_RPAREN, strdup(")"), io_number);
	(*line)++;
	return (tok);
}

/**
 * @param line the string of operator
 * @param io_number the io number to redirect
 * @brief create a new t_list *node with a t_token *content
 * @details create a TOK_PIPE or a TOK_OR
 * @return t_list *
 */
static t_list	*tok_vertical_line(const char **line, int io_number)
{
	t_list	*tok;

	if (*((*line) + 1) && *((*line) + 1) == '|')
	{
		tok = token_new(TOK_OR, strdup("||"), io_number);
		(*line) += 2;
	}
	else
	{
		tok = token_new(TOK_PIPE, strdup("|"), io_number);
		(*line)++;
	}
	return (tok);
}

/**
 * @param line the string of operator
 * @param io_number the io number to redirect
 * @brief create a new t_list *node with a t_token *content
 * @details create a TOK_AMPERSAND or a TOK_AND
 * @return t_list *
 */
static t_list	*tok_ampersand(const char **line, int io_number)
{
	t_list	*tok;

	if (*((*line) + 1) && *((*line) + 1) == '&')
	{
		tok = token_new(TOK_AND, strdup("&&"), io_number);
		(*line) += 2;
	}
	else
	{
		tok = token_new(TOK_AMPERSAND, strdup("&"), io_number);
		(*line)++;
	}
	return (tok);
}

/**
 * @param line the string of operator
 * @param io_number the io number to redirect
 * @brief create a new t_list *node with a t_token *content
 * @details create a TOK_HEREDOC, a TOK_REDIR_DUP_IN or a TOK_REDIR_IN
 * @return t_list *
 */
static t_list	*tok_redir_in(const char **line, int io_number)
{
	t_list	*tok;

	if (*((*line) + 1) && *((*line) + 1) == '<')
	{
		tok = token_new(TOK_HEREDOC, strdup("<<"), io_number);
		(*line) += 2;
	}
	else if (*((*line) + 1) && *((*line) + 1) == '&')
	{
		tok = token_new(TOK_REDIR_DUP_IN, strdup("<&"), io_number);
		(*line) += 2;
	}
	else
	{
		tok = token_new(TOK_REDIR_IN, strdup("<"), io_number);
		(*line)++;
	}
	return (tok);
}

/**
 * @param line the string of operator
 * @param io_number the io number to redirect
 * @brief create a new t_list *node with a t_token *content
 * @details create a TOK_REDIR_APPEND, a TOK_REDIR_DUP_OUT or a TOK_REDIR_OUT
 * @return t_list *
 */
static t_list	*tok_redir_out(const char **line, int io_number)
{
	t_list	*tok;

	if (*((*line) + 1) && *((*line) + 1) == '>')
	{
		tok = token_new(TOK_REDIR_APPEND, strdup(">>"), io_number);
		(*line) += 2;
	}
	else if (*((*line) + 1) && *((*line) + 1) == '&')
	{
		tok = token_new(TOK_REDIR_DUP_OUT, strdup(">&"), io_number);
		(*line) += 2;
	}
	else
	{
		tok = token_new(TOK_REDIR_OUT, strdup(">"), io_number);
		(*line)++;
	}
	return (tok);
}

/*
 * @param the string of the operator
 * @brief extract the file descriptor before the operator
 * @return an int
 */
static int	extract_io_number(const char **line)
{
	int	io_number;

	io_number = -1;
	if (isdigit(**line))
	{
		io_number = atoi(*line);
		while (isdigit(**line))
			(*line)++;
	}
	return (io_number);
}

/*
 * @param the string of the operator
 * @brief read the operator and tokenize it
 * @details choose the good function to tokenize
 * @return a t_list* node
 */
t_list	*read_operator(const char **line)
{
	t_list	*tok;
	t_list	*(*operator_tokenize[8])(const char **, int);
	char	*operators;
	int		i;
	int		io_number;

	io_number = extract_io_number(line);
	operator_tokenize[0] = &tok_semicolon;
	operator_tokenize[1] = &tok_newline;
	operator_tokenize[2] = &tok_lparen;
	operator_tokenize[3] = &tok_rparen;
	operator_tokenize[4] = &tok_vertical_line;
	operator_tokenize[5] = &tok_ampersand;
	operator_tokenize[6] = &tok_redir_in;
	operator_tokenize[7] = &tok_redir_out;
	i = 0;
	operators = ";\n()|&<>";
	while (operators[i] && operators[i] != **line)
		i++;
	if (!operators[i])
		return (NULL);
	tok = operator_tokenize[i](line, io_number);
	return (tok);
}
