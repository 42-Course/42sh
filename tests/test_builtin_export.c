/**
 * @file test_builtin_export.c
 * @brief Unit tests for the 42sh export builtin.
 */

#include "minunit.h"
#include "builtins.h"
#include "variables.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief Initialize a fresh shell for testing
 */
static void	export_test_init(t_shell *shell)
{
	memset(shell, 0, sizeof(*shell));
	shell->env_dirty = 1;
}

/**
 * @brief Test export with no arguments (display exported variables)
 */
static void	test_export_no_args(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", NULL};

	export_test_init(&shell);
	var_set(&shell, "VAR1", "value1");
	var_export(&shell, "VAR1");
	var_set(&shell, "VAR2", "value2");

	ret = builtin_export(&shell, 1, argv);
	MU_ASSERT_INT(0, ret);
}

/**
 * @brief Test export with existing variable name only
 */
static void	test_export_existing_var(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "MYVAR", NULL};
	t_var	*var;

	export_test_init(&shell);
	var_set(&shell, "MYVAR", "value");
	ret = builtin_export(&shell, 2, argv);
	var = var_get(&shell, "MYVAR");

	MU_ASSERT_INT(0, ret);
	MU_ASSERT_INT(1, var->exported);
}

/**
 * @brief Test export with assignment (set and export)
 */
static void	test_export_assignment(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "NEWVAR=newvalue", NULL};
	t_var	*var;
	char	*value;

	export_test_init(&shell);
	ret = builtin_export(&shell, 2, argv);
	var = var_get(&shell, "NEWVAR");
	value = var_get_value(&shell, "NEWVAR");

	MU_ASSERT_INT(0, ret);
	MU_ASSERT_STR("should be newvalue","newvalue", value);
	MU_ASSERT_INT(1, var->exported);
}

/**
 * @brief Test export with multiple assignments
 */
static void	test_export_multiple_assignments(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "VAR1=val1", "VAR2=val2", "VAR3=val3", NULL};

	export_test_init(&shell);
	ret = builtin_export(&shell, 4, argv);

	MU_ASSERT_INT(0, ret);
	MU_ASSERT_INT(1, var_get(&shell, "VAR1")->exported);
	MU_ASSERT_INT(1, var_get(&shell, "VAR2")->exported);
	MU_ASSERT_INT(1, var_get(&shell, "VAR3")->exported);
}

/**
 * @brief Test export with mixed assignments and variable names
 */
static void	test_export_mixed(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "EXISTING=newval", "PATH", NULL};

	export_test_init(&shell);
	var_set(&shell, "PATH", "/usr/bin");
	ret = builtin_export(&shell, 3, argv);

	MU_ASSERT_INT(0, ret);
	MU_ASSERT_INT(1, var_get(&shell, "EXISTING")->exported);
	MU_ASSERT_INT(1, var_get(&shell, "PATH")->exported);
}

/**
 * @brief Test export with empty value assignment
 */
static void	test_export_empty_value(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "EMPTY_VAR=", NULL};
	t_var	*var;

	export_test_init(&shell);
	ret = builtin_export(&shell, 2, argv);
	var = var_get(&shell, "EMPTY_VAR");

	MU_ASSERT_INT(0, ret);
	MU_ASSERT_STR("should be empty","", var_get_value(&shell, "EMPTY_VAR"));
	MU_ASSERT_INT(1, var->exported);
}

/**
 * @brief Test export with invalid identifier (starting with digit)
 */
static void	test_export_invalid_digit(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "2INVALID", NULL};

	export_test_init(&shell);
	ret = builtin_export(&shell, 2, argv);

	MU_ASSERT_INT(1, ret);
}

/**
 * @brief Test export with invalid assignment (digit start)
 */
static void	test_export_invalid_assign(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "2INVALID=value", NULL};

	export_test_init(&shell);
	ret = builtin_export(&shell, 2, argv);

	MU_ASSERT_INT(1, ret);
}

/**
 * @brief Test export handling of equals in value
 */
static void	test_export_equals_in_value(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "CONFIG=key=value", NULL};

	export_test_init(&shell);
	ret = builtin_export(&shell, 2, argv);

	MU_ASSERT_INT(0, ret);
	MU_ASSERT_STR("should be key=value","key=value", var_get_value(&shell, "CONFIG"));
}

/**
 * @brief Test export with special characters
 */
static void	test_export_special_chars(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "PATH=/usr/bin:/bin", "_PRIVATE=secret", NULL};

	export_test_init(&shell);
	ret = builtin_export(&shell, 3, argv);

	MU_ASSERT_INT(0, ret);
	MU_ASSERT_STR("should be /usr/bin:/bin","/usr/bin:/bin", var_get_value(&shell, "PATH"));
	MU_ASSERT_STR("should be secret","secret", var_get_value(&shell, "_PRIVATE"));
}

/**
 * @brief Test export marks non-exported variable as exported
 */
static void	test_export_marks_as_exported(void)
{
	t_shell	shell;
	int		ret;
	char	*argv[] = {"export", "UNEXPORTED", NULL};
	t_var	*var;

	export_test_init(&shell);
	var_set(&shell, "UNEXPORTED", "value");
	var = var_get(&shell, "UNEXPORTED");
	MU_ASSERT_INT(0, var->exported);

	ret = builtin_export(&shell, 2, argv);
	var = var_get(&shell, "UNEXPORTED");
	MU_ASSERT_INT(0, ret);
}

/**
 * @brief Test export with NULL shell
 */
static void	test_export_null_shell(void)
{
	int		ret;
	char	*argv[] = {"export", "VAR=value", NULL};

	ret = builtin_export(NULL, 2, argv);
	MU_ASSERT_INT(1, ret);
}

/**
 * @brief Test export with NULL argv
 */
static void	test_export_null_argv(void)
{
	t_shell	shell;
	int		ret;

	export_test_init(&shell);
	ret = builtin_export(&shell, 2, NULL);
	MU_ASSERT_INT(1, ret);
}

/**
 * @brief Run the full test suite for the export builtin
 */
void	test_builtin_export_suite(void)
{
	test_export_no_args();
	test_export_existing_var();
	test_export_assignment();
	test_export_multiple_assignments();
	test_export_mixed();
	test_export_empty_value();
	test_export_invalid_digit();
	test_export_invalid_assign();
	test_export_equals_in_value();
	test_export_special_chars();
	test_export_marks_as_exported();
	test_export_null_shell();
	test_export_null_argv();
}
