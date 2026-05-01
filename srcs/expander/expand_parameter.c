#include "expander.h"

char **expand_word_to_fields(struct s_shell *shell, const char *word) {
	
	char *expanded = expand_dollar(shell, (char *)word, 0, false);
	
	char *tilde_expanded = expand_tilde(shell, expanded, 0);
	
	free(expanded);
	
	char **fields = ft_split(tilde_expanded, ' ');
	
	free(tilde_expanded);
	
	return fields;
}

char *expand_dollar(t_shell * shell, char *input, size_t pos, bool in_double_quote) {
	(void) pos;
	(void) in_double_quote;
	
	if (input[0] == '$') {

		char * var_name = input + 1;
		char * var_value = var_get_value(shell, var_name);

		if (var_value) {
			return strdup(var_value);
		} else {
			return strdup("");
		}
	}

	return strdup(input);
}

char *expand_braced_variable(t_shell * shell, char *input, size_t pos, bool in_double_quote) {
	(void) pos;
	(void) in_double_quote;

	if (input[0] == '$' && input[1] == '{') {

		char * var_name = input + 2;
		char * closing_brace = strchr(var_name, '}');

		if (closing_brace) {
			*closing_brace = '\0';
			char * var_value = var_get_value(shell, var_name);
			if (var_value) {
				return strdup(var_value);
			} else {
				return strdup("");
			}
		}
	}

	return strdup(input);
}
