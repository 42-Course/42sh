# include "42sh.h"
# include "expander.h"

// static char * expand_word_internal(t_shell *shell, char * input) {
    // char *  result = NULL;
    // size_t  pos = 0;
    // bool    in_single_quote = false;
    // bool    in_double_quote = false;
// 
    // while (input[pos]) {
        // const char c = input[pos];
// 
        // if (c == '\'' && !in_double_quote) {
            // in_single_quote = true;
            // pos++;
            // continue ;
        // }
        // 
        // if (c == '"' && !in_single_quote) {
            // in_double_quote = true;
            // pos++;
            // continue;
        // }
// 
        // if (in_single_quote) {
            // realloc(result, strlen(result + 2));
            // strcat(result, &c);
            // pos++;
            // continue;
        // }
    // }
// 
    // return result;
// }

// char * expand_word(t_shell * shell, const char * word)