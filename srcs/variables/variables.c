/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   variables.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wengzhang <marvin@42.fr>                   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 00:00:00 by wengzhang         #+#    #+#             */
/*   Updated: 2026/04/29 21:40:49 by jguillem         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "42sh.h"
#include "variables.h"

/* TODO P3: implement variable storage (linked list of t_var) */

t_var	*var_get(t_shell *shell, const char *name) {
	t_list *temp = shell->variables;

	while (temp) {

		t_var * var_temp = (t_var *)temp->content;

		if (!strcmp((const char*)var_temp->name, name)) {
			return var_temp;
		}

		temp = temp->next;
	}

	return (NULL);
}


char	*var_get_value(t_shell *shell, const char *name) {
	t_list *temp = shell->variables;

	t_var * var_temp = NULL;
	while (temp) {

		var_temp = (t_var *)temp->content;

		if (!strcmp((const char*)var_temp->name, name)) {
			return var_temp->value;
		}

		var_temp = NULL;
		temp = temp->next;
	}

	return (NULL);
}

int	var_set(t_shell *shell, const char *name, const char *value) {
	t_var * temp = var_get(shell, name);

	if (temp) {
		if (temp->readonly) {
			return (1);
		}
		if (temp->value) {
			free(temp->value);
		}
		if (value)
			temp->value = ft_strdup(value);
		else
			temp->value = NULL;

	} else {
		temp = malloc(sizeof(t_var ));
		
		if (!temp) {
			return (1);
		}

		temp->name = ft_strdup(name);
		temp->value = value ? ft_strdup(value) : NULL;

		temp->exported = 0;
		temp->readonly = 0;

		t_list *new_node = ft_lstnew((void*)temp);
		if (!new_node) {
			free(temp);
			return (1);
		}
		ft_lstadd(&shell->variables, new_node);
	}

	shell->env_dirty = 1;
	return (0);
}

int	var_unset(t_shell *shell, const char *name) {
	if (!shell->variables) {
		return (0);
	}

	if (strcmp(((t_var *)shell->variables->content)->name,name)  == 0) {
			
			if (((t_var*)shell->variables->content)->readonly) {
				return  (1);
			}

			t_list *tmp = shell->variables;
			shell->variables = tmp->next;

			t_var *temp_var = (t_var *)tmp->content;
			if (temp_var->name) free(temp_var->name);
			if(temp_var->value)	free(temp_var->value);

			free(temp_var);
			free(tmp);
			
			shell->env_dirty = 1;
			return (0);
	}
	
	t_list *temp = shell->variables ? shell->variables : NULL;
	t_list *temp_next = temp->next ? temp->next : NULL;

	while (temp_next) {

		t_var * var_temp = (t_var *)temp_next->content;

		if (!strcmp((const char*)var_temp->name, name)) {
			if (var_temp->readonly) {
				return (1);
			}

			temp->next = temp_next->next;
			temp_next->next = NULL;

			free(var_temp->name);
			free(var_temp->value);
			free(var_temp);
			free(temp_next);
			
			shell->env_dirty = 1;
			return (0);
		}

		temp = temp_next;
		temp_next = temp_next->next;
	}

	shell->env_dirty = 1;
	return (0);
}

int	var_export(t_shell *shell, const char *name) {
	t_var * temp = NULL;

	temp = var_get(shell, name);

	if (!temp) {
		
		temp = malloc(sizeof(t_var));
		if (!temp) {
			return (1);
		}

		temp->name = ft_strdup(name);
		temp->value = NULL;
		temp->readonly = 0;
		temp->exported = 1;

		t_list *new_node = ft_lstnew(temp);
		if (!new_node) {
			free(temp);
			return (1);
		}
		ft_lstadd(&shell->variables, new_node);

	} else {
		temp->exported = 1;
	}

	shell->env_dirty = 1;
	return (0);
}

static void free_env(char **env) {
	if (!env) {
		return ;
	}

	for (size_t i = 0; env[i] != NULL ; i++) {
		free(env[i]);
	} free(env);

	return ;
}

char **var_get_environ(t_shell *shell) {
    if (!shell->env_dirty)
        return shell->env;

    if (shell->env)
        free_env(shell->env);

    size_t count = 0;
    t_list *tmp = shell->variables;

    while (tmp)
    {
        if (((t_var*)tmp->content)->exported)
            count++;
        tmp = tmp->next;
    }

    char **env = malloc(sizeof(char*) * (count + 1));
    if (!env)
        return NULL;

    tmp = shell->variables;
    size_t i = 0;

    while (tmp)
    {
        t_var *v = (t_var*)tmp->content;

        if (v->exported)
        {
            size_t len = strlen(v->name) + (v->value ? strlen(v->value) : 0) + 2;
            env[i] = malloc(len);
            if (!env[i])
                return free_env(env), NULL;

            snprintf(env[i], len, "%s=%s", v->name, v->value ? v->value : "");
            i++;
        }
        tmp = tmp->next;
    }

    env[i] = NULL;

    shell->env = env;
    shell->env_dirty = 0;
    return env;
}

static int set_name(char **dst, char *src, int *position) {

	size_t start_pos = *position;
	size_t curr_pos = start_pos;

	while (src[curr_pos] && src[curr_pos] != '=') {
		curr_pos++;
	}

	*dst = malloc(curr_pos - start_pos + 1);
	if (!*dst) {
		return (-1);
	}
	memcpy(*dst, &src[start_pos], curr_pos - start_pos);
	(*dst)[curr_pos - start_pos] = '\0';

	if (src[curr_pos] == '=')
		curr_pos++;
	
	*position = curr_pos;
	return (0);
}

static int set_value(char **dst, char *src, int *position) {

	size_t start_pos = *position;
	size_t curr_pos = start_pos;

	while (src[curr_pos]) {
		curr_pos++;
	}

	*dst = malloc(curr_pos - start_pos + 1);
	if (!*dst) {
		return (-1);
	}

	memcpy(*dst, &src[start_pos], curr_pos - start_pos);

	(*dst)[curr_pos - start_pos] = '\0';
	*position = curr_pos;
	return (0);
}

void	var_init_from_environ(t_shell *shell, char **envp) {
	
	int position = 0;
	char *name = NULL;
	char *value = NULL;
	
	for (size_t i = 0; envp[i] != NULL; i++) {
		position = 0;
		name = NULL;
		value = NULL;

		
		if (set_name(&name,envp[i], &position)) {
			if (shell->env)	free_env(shell->env);
			break ;
		}


		if (set_value(&value,envp[i], &position)) {
			if (name)		free(name);
			if (shell->env) free_env(shell->env);
			break;
		}

		if (var_set(shell, name, value)) {
			if(name)		free(name);
			if(value)		free(value);
			if (shell->env) free_env(shell->env);
			break ;
		}

		if (var_export(shell, name)) {
			if(name) 		free(name);
			if(value)		free(value);
			if(shell->env)	free(shell->env);
			break ;
		}

		if (name) free(name);
		if (value) free(value);

	}
	
	return ;
}
