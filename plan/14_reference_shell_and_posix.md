# Reference Shell & POSIX Strategy

## Why Pick a Reference Shell?

A shell has thousands of micro-behaviors the subject doesn't define. Your reference shell is your **answer key** - when you don't know what `42sh` should do, run the command in your reference shell and copy that behavior.

```bash
# Examples of ambiguous behavior only a reference shell answers:
echo hello | cd /tmp         # Does cd work in a pipe?
export =                     # Error? Silent ignore?
cd ""                        # Error or go to HOME?
echo "$"                     # Print $ or empty?
```

**Our reference shell: bash**

## POSIX vs Bash

**POSIX** is a written specification defining the minimum behavior every `/bin/sh` must support. It's strict but limited.

**Bash** implements everything POSIX requires, plus many extensions:

| Feature | POSIX | Bash |
|---------|-------|------|
| `echo -n` | Undefined | Works |
| `export VAR=val` | Allowed but optional | Works |
| `[[ ]]` extended test | No | Yes |
| `${var:offset:len}` | No | Yes |
| `<<<` here-string | No | Yes |
| `<()` process subst. | No | Yes |

## What the Subject Says

1. *"Choose a reference shell and replicate its basic behaviour"* - Use bash as your answer key.
2. *"In case of doubt, refer to the POSIX standard"* - POSIX is the tiebreaker when bash behavior is confusing.
3. *"Acceptable to implement differently if consistent"* - You can deviate, but be internally consistent.

**The subject does NOT require POSIX compliance.** Full POSIX compliance is listed as a **bonus**.

## Our Strategy

1. **Day-to-day**: Test against `bash`. If `42sh` matches `bash`, we're good.
2. **Parser/grammar**: Follow the POSIX grammar structure - it's well-defined and correct.
3. **Builtins**: When the subject says "POSIX options", check what POSIX requires for that specific builtin.
4. **Don't stress about strict compliance** - evaluators will type commands they know from bash, not read the POSIX spec during defense.
