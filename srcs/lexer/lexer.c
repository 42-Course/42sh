/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jguillem <jguillem@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 13:30:36 by jguillem          #+#    #+#             */
/*   Updated: 2026/02/28 19:19:24 by jguillem         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"

t_list	*lexer_tokenize(const char *input)
{
	t_list	*head;
	t_list	*token;
	char	*eof;

	head = NULL;
	token = NULL;
	while (*input)
	{
		while (isspace(*input))
			input++;
		if (!*input)
			break ;
		if (is_operator_start(input))
			token = read_operator(&input);
		else
			token = read_word(&input);
		ft_lstappend(&head, token);
	}
	eof = strdup("");
	ft_lstappend(&head, token_new(TOK_EOF, eof, -1));
	return (head);
}
