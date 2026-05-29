/**
 * @file signal_child.c
 * @brief Signal handling setup for child processes in 42sh.
 * @author wengzhang, pulgamecanica
*/

#include "signals.h"
#include <stddef.h>

/**
 * @details After fork the child inherits the parent's signal dispositions (SIG_IGN
 *          for interactive mode).  We must reset them to SIG_DFL so the executed
 *          program responds to signals normally.
 *          SIGPIPE is reset alongside the others: this matches dash/ash and gives
 *          children a predictable disposition regardless of how the shell itself
 *          was launched -- e.g. an IDE-embedded terminal or CI runner that sets
 *          SIGPIPE=SIG_IGN upstream cannot otherwise sneak that into our children
 *          and cause SIGPIPE-targeted probes (signals.sh's kill -PIPE case) to
 *          silently no-op. (bash leaves it inherited; we deliberately diverge.)
 *          The pipeline-test stderr-vs-bash comparison still needs a canonical
 *          SIGPIPE environment for symmetry; the test side handles that via a
 *          perl trampoline in b_case (tests/integration/posix_baseline.sh).
 */
void	signals_setup_child(void)
{
	struct sigaction	sa;

	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_DFL;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGTSTP, &sa, NULL);
	sigaction(SIGTTIN, &sa, NULL);
	sigaction(SIGTTOU, &sa, NULL);
}
