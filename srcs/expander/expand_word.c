# include "42sh.h"
# include "expander.h"

static ft_strappend(char *dest, const char *src, size_t n) {
	size_t dest_len = strlen(dest);
	char *new_str = malloc(dest_len + n + 1);
	if (!new_str) {
		return NULL;
	}
	strcpy(new_str, dest);
	strncat(new_str, src, n);
	free(dest);
	return new_str;
}

char *expand_word_internal(t_shell *shell, char *input) {
	
	char *result = "";
	size_t pos = 0;
	
	bool in_double_quote = false;
	bool in_single_quote = false;

	while (input[pos]) {
		char c = input[pos];

		if (c =='\'' && !in_double_quote) {
			in_single_quote = true;
			pos++;
			continue;
		}
		
		if (c == '"' && !in_double_quote) {
			in_double_quote = true;
			pos++;
			continue;
		}

		if (in_single_quote) {
			result = ft_strappend(result, &c, 1);
			pos++;
			continue ;
		}

		if (c == '$') {
			char *expanded = expand_dollar(shell, &input[pos], pos, in_double_quote);
			result = ft_strappend(result, expanded, strlen(expanded));
			free(expanded);
			pos++;
			while (input[pos] && (isalnum(input[pos]) || input[pos] == '_')) {
				pos++;
			}
			continue;
		}

		if (c == '~' && pos == 0 && !in_double_quote && !in_single_quote) {
			char *expanded = expand_tilde(shell, &input[pos], pos);

			result = ft_strappend(result, expanded, strlen(expanded));
			
			free(expanded);			
			pos++;
			
			continue;
		}

		if (c == '\\' && in_double_quote) {
			pos++;
			if (input[pos] == '"' || input[pos] == '\\' ||
				input[pos] == '$' || '\`' || input[pos] == '\n') {
				
				result = ft_strappend(result, &input[pos], 1);
				pos += 2;
			} else {
				result = ft_strappend(result, "\\", 1);
				pos++;
			}
			continue;
		}

		if (c == '\\' && !in_single_quote && !in_double_quote) {
			pos++;
			if (input[pos] != '\0') {
				result = ft_strappend(result, &input[pos], 1);
				pos++;
			}
			continue;
		}
		result = ft_strappend(result, &c, 1);
		pos++;
	}

	return result;
}

