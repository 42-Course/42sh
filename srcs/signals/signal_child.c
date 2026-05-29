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
 *          SIGPIPE is intentionally NOT reset here: POSIX says SIG_IGN inherited
 *          across exec must be preserved, and bash matches that. Forcing SIG_DFL
 *          here would diverge on hosts that ignore SIGPIPE upstream (notably the
 *          GitHub Actions runner, written in node.js, which sets SIG_IGN before
 *          spawning user steps), making coreutils-9.x children die silently here
 *          while bash's children emit "Broken pipe" -- breaking pipeline tests
 *          that diff our stderr against bash's.
 */
void	signals_setup_child(void)
{
	struct sigaction	sa;

	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = SIG_DFL;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGTSTP, &sa, NULL);
	sigaction(SIGTTIN, &sa, NULL);
	sigaction(SIGTTOU, &sa, NULL);
}
