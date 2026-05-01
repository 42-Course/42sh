# include "expander.h"

char *expand_arithmetic(t_shell *shell, char *input, size_t pos) {
	char *result = NULL;
	size_t start = 0;
	size_t end = 0;
	int closing_parentheses = 0;

	while (input[pos + start] && input[pos + start] == '$' || input[pos + start] == '(' || input[pos + start] == ')' || isdigit(input[pos + start]) || isspace(input[pos + start])) {
		start++;
	}

	end = start;

	while (input[end]) {
		if (input[end] == ')') {
			closing_parentheses++;
		}
		if (closing_parentheses == 2) {
			end -= 2;
			break;
		}
		end++;
	}

	if (closing_parentheses != 2) {
		return strdup(input);
	}


	return result;
}