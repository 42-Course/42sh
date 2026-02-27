/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_words.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jguillem <jguillem@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/24 16:08:04 by jguillem          #+#    #+#             */
/*   Updated: 2026/02/27 18:06:03 by jguillem         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ctype.h>
#include <string.h>
#include "lexer.h"

static void	toggle(int *flag)
{
	if (*flag != 0)
		*flag = 0;
	else
		*flag = 1;
}

t_list	*read_word(const char **line)
{
	t_list		*tok;
	const char	*scout;
	int			in_squote;
	int			in_dquote;

	in_squote = 0;
	in_dquote = 0;
	scout = *line;
	while (*scout
		&& (in_squote || in_dquote
			|| (!isspace(*scout) && !is_operator(*scout))))
	{
		if (*scout == '\'' && !in_dquote)
			toggle(&in_squote);
		else if (*scout == '"' && !in_squote)
			toggle(&in_dquote);
		else if (*scout == '\\' && !in_squote)
			if (*(scout + 1))
				scout++;
		scout++;
	}
	tok = token_new(TOK_WORD, strndup(*line, scout - *line), -1);
	(*line) = scout;
	return (tok);
}
