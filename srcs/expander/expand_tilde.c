# include "expander.h"

char * expand_tilde(t_shell * shell, char * input, size_t pos) {
	(void) shell;
	(void) pos;
	if (input[0] == '~') {
		char * home = getenv("HOME");
		if (home) {
			char * result = malloc(strlen(home) + strlen(input));
			if (!result) {
				return NULL;
			}
			strcpy(result, home);
			strcat(result, input + 1);
			return result;
		}
	}
	return strdup(input);
}