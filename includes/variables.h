/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   variables.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 21:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/02/24 00:00:00 by pulgamecanica    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VARIABLES_H
# define VARIABLES_H

/*
** Shell variable data node.
** Stored in t_shell.variables as t_list* (each node->content is a t_var*).
** No *next field — traversal is done via the t_list wrapper in t_shell.
*/
typedef struct s_var
{
	char	*name;
	char	*value;
	int		exported;
	int		readonly;
}	t_var;

/*
** Alias data node.
** Stored in t_shell.aliases as t_list* (each node->content is a t_alias*).
*/
typedef struct s_alias
{
	char	*name;
	char	*value;
}	t_alias;

#endif
