#ifndef TEST_VARIABLES

# include "minunit.h"
# include "variables.h"
# include "42sh.h"

/*
typedef struct s_var
{
    char	*name;
    char	*value;
    int		exported;
    int		readonly;
}	t_var;

*/

extern void	stub_shell_init(t_shell *shell);
extern void	stub_shell_cleanup(t_shell *shell);

// ========== var_get ==========
void test_var_get_returns_null_for_nonexistent(t_shell *shell) {
    t_var *var1 = NULL;

    var1 = var_get(shell, "NONEXISTENT_VAR");
    MU_ASSERT("Variable should be NULL for nonexistent var", var1 == NULL);
}

void test_var_get_returns_existing_variable(t_shell *shell, char *name, char *expected_value) {
    t_var *var1 = NULL;

    var1 = var_get(shell, name);
    MU_ASSERT("Variable should not be NULL", var1 != NULL);
    MU_ASSERT_STR("Variable value should match", var1->value, expected_value);
}

// ========== var_get_value ==========
void test_var_get_value_returns_null_for_nonexistent(t_shell *shell) {
    char *value = NULL;

    value = var_get_value(shell, "NONEXISTENT_VAR");
    MU_ASSERT("Value should be NULL for nonexistent var", value == NULL);
}

void test_var_get_value_returns_correct_value(t_shell *shell, char *name, char *expected_value) {
    char *value = NULL;

    value = var_get_value(shell, name);
    MU_ASSERT("Value should not be NULL", value != NULL);
    MU_ASSERT_STR("Value should match expected", value, expected_value);
}

void test_variable_get_value(t_shell *shell, char *name) {
    t_var *var1 = NULL;
    char *value = NULL;

    var1 = var_get(shell, name);
    value = var_get_value(shell, name);
    MU_ASSERT("Value should not be NULL", value != NULL);
    MU_ASSERT_STR("Value should match the value provided by var", var1->value, value);
}

// ========== var_set ==========
void test_var_set_creates_new_variable(t_shell *shell, char *name, char *value) {
    int result = 0;
    t_var *var = NULL;

    result = var_set(shell, name, value);
    MU_ASSERT("var_set should return 0 on success", result == 0);
    
    var = var_get(shell, name);
    MU_ASSERT("Variable should exist after var_set", var != NULL);
    MU_ASSERT_STR("Variable value should match", var->value, value);
}

void test_var_set_updates_existing_variable(t_shell *shell, char *name, char *new_value) {
    int result = 0;
    t_var *var = NULL;

    result = var_set(shell, name, new_value);
    MU_ASSERT("var_set should return 0 on success", result == 0);
    
    var = var_get(shell, name);
    MU_ASSERT_STR("Variable value should be updated", var->value, new_value);
}

void test_var_set_with_null_value(t_shell *shell, char *name) {
    int result = 0;
    t_var *var = NULL;

    result = var_set(shell, name, NULL);
    MU_ASSERT("var_set should return 0 on success", result == 0);
    
    var = var_get(shell, name);
    MU_ASSERT("Variable should exist", var != NULL);
    MU_ASSERT("Variable value should be NULL", var->value == NULL);
}

void test_var_set_sets_env_dirty_flag(t_shell *shell, char *name, char *value) {
    shell->env_dirty = 0;
    var_set(shell, name, value);
    MU_ASSERT("env_dirty flag should be set", shell->env_dirty == 1);
}

void test_variable_exist_and_has_value(t_shell *shell, char *name, char *value) {
    t_var *var1 = NULL;

    var1 = var_get(shell, name);
    MU_ASSERT("Variable should not be null", var1 != NULL);
    MU_ASSERT_STR("Variable should match the value provided", var1->value, value);	
}

// ========== var_unset ==========
void test_var_unset_removes_variable(t_shell *shell, char *name) {
    int result = 0;
    t_var *var = NULL;

    result = var_unset(shell, name);
    MU_ASSERT("var_unset should return 0 on success", result == 0);
    
    var = var_get(shell, name);
    MU_ASSERT("Variable should not exist after var_unset", var == NULL);
}

void test_var_unset_nonexistent_variable(t_shell *shell) {
    int result = 0;

    result = var_unset(shell, "NONEXISTENT_VAR");
    MU_ASSERT("var_unset should return 0 for nonexistent var", result == 0);
}

void test_var_unset_sets_env_dirty_flag(t_shell *shell, char *name) {
    shell->env_dirty = 0;
    var_unset(shell, name);
    MU_ASSERT("env_dirty flag should be set", shell->env_dirty == 1);
}

// ========== var_export ==========
void test_var_export_exports_existing_variable(t_shell *shell, char *name) {
    int result = 0;
    t_var *var = NULL;

    result = var_export(shell, name);
    MU_ASSERT("var_export should return 0 on success", result == 0);
    
    var = var_get(shell, name);
    MU_ASSERT("Variable should be exported", var->exported == 1);
}

void test_var_export_creates_and_exports_nonexistent(t_shell *shell, char *name) {
    int result = 0;
    t_var *var = NULL;

    result = var_export(shell, name);
    MU_ASSERT("var_export should return 0 on success", result == 0);
    
    var = var_get(shell, name);
    MU_ASSERT("Variable should exist", var != NULL);
    MU_ASSERT("Variable should be exported", var->exported == 1);
    MU_ASSERT("Variable value should be NULL", var->value == NULL);
}

void test_var_export_sets_env_dirty_flag(t_shell *shell, char *name) {
    shell->env_dirty = 0;
    var_export(shell, name);
    MU_ASSERT("env_dirty flag should be set", shell->env_dirty == 1);
}

// ========== var_get_environ ==========
void test_var_get_environ_returns_exported_only(t_shell *shell) {
    char **env = NULL;
    int count = 0;

    env = var_get_environ(shell);
    MU_ASSERT("environ should not be NULL", env != NULL);
    
    while (env[count] != NULL) {
        count++;
    }
    
    MU_ASSERT("environ should have exported variables", count > 0);
}

void test_var_get_environ_clears_dirty_flag(t_shell *shell) {
    shell->env_dirty = 1;
    var_get_environ(shell);
    MU_ASSERT("env_dirty flag should be cleared", shell->env_dirty == 0);
}

void test_var_get_environ_format_is_name_equals_value(t_shell *shell, char *name, char *value) {
    char **env = NULL;
    int i = 0;
    int found = 0;
    char expected[256];

    snprintf(expected, sizeof(expected), "%s=%s", name, value);
    env = var_get_environ(shell);
    
    while (env[i] != NULL) {
        if (strcmp(env[i], expected) == 0) {
            found = 1;
            break;
        }
        i++;
    }
    
    MU_ASSERT("Variable should be in environ with correct format", found == 1);
}

// ========== var_init_from_environ ==========
void test_var_init_from_environ_parses_correctly(t_shell *shell) {
    char *envp[] = {"TEST_INIT=value1", "ANOTHER=value2", NULL};
    t_var *var1 = NULL;
    t_var *var2 = NULL;

    var_init_from_environ(shell, envp);
    
    var1 = var_get(shell, "TEST_INIT");
    var2 = var_get(shell, "ANOTHER");
    
    MU_ASSERT("First variable should exist", var1 != NULL);
    MU_ASSERT_STR("First variable value should match", var1->value, "value1");
    MU_ASSERT("Second variable should exist", var2 != NULL);
    MU_ASSERT_STR("Second variable value should match", var2->value, "value2");
}

void test_var_init_from_environ_handles_empty_value(t_shell *shell) {
    char *envp[] = {"EMPTY=", NULL};
    t_var *var = NULL;

    var_init_from_environ(shell, envp);
    
    var = var_get(shell, "EMPTY");
    MU_ASSERT("Variable should exist", var != NULL);
    MU_ASSERT("Variable value should be empty string", var->value != NULL && strlen(var->value) == 0);
}

// ========== Helper for initial tests ==========
void test_if_shell_do_not_exit(t_shell * shell, char *name) {
    t_var *var1 = NULL;

    var1 = var_get(shell, name);
    MU_ASSERT("Variable should be NULL", var1 == NULL);
}

// ========== Main test suite ==========
void	test_variables_suite(void) {
    t_shell shell = {0};

    // Test initial state
    test_if_shell_do_not_exit(&shell, "PATH");
    stub_shell_init(&shell);
    test_if_shell_do_not_exit(&shell, "PATH");

    // Test var_set - create new
    test_var_set_creates_new_variable(&shell, "PATH", "/usr/bin");
    test_var_set_sets_env_dirty_flag(&shell, "VAR1", "value1");

    // Test var_get and var_get_value - existing
    test_variable_exist_and_has_value(&shell, "PATH", "/usr/bin");
    test_variable_get_value(&shell, "PATH");
    test_var_get_returns_existing_variable(&shell, "PATH", "/usr/bin");
    test_var_get_value_returns_correct_value(&shell, "PATH", "/usr/bin");

    // Test var_set - update existing
    test_var_set_updates_existing_variable(&shell, "PATH", "/usr/local/bin");
    test_variable_get_value(&shell, "PATH");

    // Test var_set - null value
    test_var_set_with_null_value(&shell, "NULL_VAR");

    // Test var_get - nonexistent
    test_var_get_returns_null_for_nonexistent(&shell);
    test_var_get_value_returns_null_for_nonexistent(&shell);

    // Test var_export
    test_var_export_exports_existing_variable(&shell, "PATH");
    test_var_export_creates_and_exports_nonexistent(&shell, "EXPORTED_VAR");
    test_var_export_sets_env_dirty_flag(&shell, "ANOTHER_EXPORT");

    // Test var_get_environ
    test_var_get_environ_returns_exported_only(&shell);
    test_var_get_environ_clears_dirty_flag(&shell);
    test_var_get_environ_format_is_name_equals_value(&shell, "PATH", "/usr/local/bin");

    // Test var_unset
    test_var_unset_removes_variable(&shell, "NULL_VAR");
    test_var_get_returns_null_for_nonexistent(&shell);
    test_var_unset_nonexistent_variable(&shell);
    test_var_unset_sets_env_dirty_flag(&shell, "VAR1");

    // Test var_init_from_environ
    stub_shell_cleanup(&shell);
    stub_shell_init(&shell);
    test_var_init_from_environ_parses_correctly(&shell);
    test_var_init_from_environ_handles_empty_value(&shell);

    // Cleanup
    stub_shell_cleanup(&shell);
}

#endif