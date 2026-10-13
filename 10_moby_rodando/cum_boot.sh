#!/bin/bash
# Sessao nova + ajustes da rodada 8 + FINDQ de rastreio (so imprime o que recebe e devolve o argumento).
T=/home/claude/work/its_tape; I=$T/its.sh
$I stop 2>/dev/null; sleep 1
bash $T/boot_moby.sh || exit 1
$I send "$(cat $T/smngof_z.lisp)"; sleep 3
while read -r l; do [ -n "$l" ] && { $I send "$l"; sleep 3; }; done < $T/fixes_ambiente.lisp
[ -n "$FINDQ_DEF" ] && { $I send "${FINDQ_DEF}"; sleep 3; }
for f in $EXTRAS; do while read -r l; do [ -n "$l" ] && { $I send "$l"; sleep 3; }; done < $f; done
$I send "(SHRDLU)"; sleep 6
$I cap 25
