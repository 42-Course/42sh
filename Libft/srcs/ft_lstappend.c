/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstappend.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zweng <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/11/13 13:50:56 by zweng             #+#    #+#             */
/*   Updated: 2026/02/24 00:00:00 by pulgamecanica    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstappend(t_list **alst, t_list *node)
{
	t_list	*lstp;

	if (!alst || !node)
		return ;
	if (!(*alst))
	{
		*alst = node;
		return ;
	}
	lstp = *alst;
	while (lstp->next)
		lstp = lstp->next;
	lstp->next = node;
}
