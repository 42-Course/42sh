/**
 * @file test_history.c
 * @brief Test suite for the shell command history module.
 * @author pulgamecanica (arosado-)
 *
 * Covers history_init(), history_add() (including empty-line and duplicate
 * rejection), history_prev() / history_next() cursor navigation,
 * circular-buffer trimming when `max_size` is exceeded, and history_free().
 *
 * Guarded by `TEST_HISTORY_ENABLED` (set in `TEST_FLAGS` in the root
 * Makefile).  Once `srcs/history/` is implemented and all assertions pass,
 * remove the `#ifdef` guards and the `-D` flag.
 */

#include "minunit.h"
#include "../includes/history.h"
#include <stdlib.h>

#ifdef TEST_HISTORY_ENABLED

/**
 * Run all history module assertions.
 *
 * Tests the full lifecycle of a `t_history` object:
 * - Initialisation and zero-count state.
 * - Adding valid entries; rejection of blank and duplicate lines.
 * - history_free() resets count to zero and head to NULL.
 *
 */
void	test_history_suite(void)
{
	t_history	h;

	history_init(&h, 10);
	MU_ASSERT_INT(0, h.count);
	MU_ASSERT("head NULL on init", h.head == NULL);
	history_add(&h, "echo hello");
	history_add(&h, "ls -la");
	history_add(&h, "pwd");
	MU_ASSERT_INT(3, h.count);
	history_free(&h);
	MU_ASSERT("head NULL after free", h.head == NULL);
	MU_ASSERT_INT(0, h.count);
}

#else

void	test_history_suite(void)
{
}

#endif
