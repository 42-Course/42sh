#!/usr/bin/env bash
# Decisive regression suite for the heredoc input-unification bug.
#
# Every case drives the COMPILED ./42sh end-to-end with a complete,
# multi-line script delivered on stdin -- so the command stream and the
# heredoc body travel through the SAME reader. The in-process unit tests
# cannot exercise this path; this is the only suite that proves the
# structural invariant: "one reader owns stdin".
#
# Usage:   bash tests/integration/heredoc.sh
# Exit:    0 if every case passes, 1 otherwise.

set -u

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SH="${SHELL_BIN:-$ROOT/42sh}"
TMP="$(mktemp -d)"
PASS=0
FAIL=0

if [ -t 1 ]; then
	G=$'\033[1;32m'; R=$'\033[1;31m'; Z=$'\033[0m'
else
	G=; R=; Z=
fi

[ -x "$SH" ] || { echo "heredoc.sh: shell binary not built: $SH" >&2; exit 2; }
trap 'rm -rf "$TMP"' EXIT

# ok NAME EXPECTED ACTUAL
ok() {
	if [ "$2" = "$3" ]; then
		PASS=$((PASS + 1))
		printf "%s PASS%s %s\n" "$G" "$Z" "$1"
	else
		FAIL=$((FAIL + 1))
		printf "%s FAIL%s %s\n" "$R" "$Z" "$1"
		printf "      expected: %q\n" "$2"
		printf "      actual:   %q\n" "$3"
	fi
}

# --- T1: heredoc body is delivered to the command (pipe input) --------
out="$(printf 'cat << EOF\nalpha\nbeta\nEOF\necho TAIL\n' | "$SH" 2>/dev/null)"
ok "T1 body delivered (pipe)" "alpha
beta
TAIL" "$out"

# --- T2: no double execution (seekable file input) --------------------
printf 'cat << EOF\nalpha\nbeta\nEOF\necho TAIL\n' > "$TMP/t2.sh"
out="$("$SH" < "$TMP/t2.sh" 2>/dev/null)"
ok "T2 no double execution (file)" "alpha
beta
TAIL" "$out"

# --- T3: heredoc body is inert -- never executed as a command ---------
probe="$TMP/t3_probe"
rm -f "$probe"
printf 'cat << EOF\ntouch %s\nEOF\necho done\n' "$probe" | "$SH" >/dev/null 2>&1
if [ -e "$probe" ]; then r=EXECUTED; else r=inert; fi
ok "T3 body inert (not run as command)" "inert" "$r"

# --- T4: the command after the heredoc runs exactly once --------------
cnt="$TMP/t4_count"
rm -f "$cnt"
printf 'cat << EOF\nx\nEOF\necho mark >> %s\n' "$cnt" > "$TMP/t4.sh"
"$SH" < "$TMP/t4.sh" >/dev/null 2>&1
n=$(wc -l < "$cnt" 2>/dev/null | tr -d ' ')
ok "T4 tail command runs once" "1" "${n:-MISSING}"

# --- T5: multiple heredocs stay in sync with the command stream -------
out="$(printf 'cat << A\nfromA\nA\ncat << B\nfromB\nB\necho TAIL\n' \
	| "$SH" 2>/dev/null)"
ok "T5 multiple heredocs in order" "fromA
fromB
TAIL" "$out"

# --- T6: heredoc feeding a pipeline -----------------------------------
out="$(printf 'cat << EOF | wc -l\nl1\nl2\nl3\nEOF\n' | "$SH" 2>/dev/null \
	| tr -d ' ')"
ok "T6 heredoc into pipeline" "3" "$out"

printf '\n%d passed, %d failed\n' "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
