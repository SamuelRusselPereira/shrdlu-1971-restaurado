#!/bin/bash
# Disco limpo do ITS (da imagem pronta) + diretorio OSHRDL copiado da fita.
T=/home/claude/work/its_tape; I=$T/its.sh
wait_for() { for k in $(seq 1 ${2:-60}); do tail -c +$((M+1)) $T/console.log | tr '\r' '\n' | grep -q "$1" && return 0; sleep 1; done; echo "TIMEOUT esperando: $1"; return 1; }
echo "=== restaura disco $(date -u)" >> $T/console.log
M=$(stat -c %s $T/console.log); $I start; wait_for "DSKDMP" 90 || exit 1; sleep 3
DELAY=0.4 $I send "its"; sleep 1; $I cr; sleep 4
DELAY=0.3 $I send "styo+2/popj p,"; sleep 1; $I cr; sleep 2; $I esc; sleep 1; $I send "g"
wait_for "IN OPERATION" 90 || exit 1; sleep 3; $I ctrl z; wait_for "Happy hacking" 30 || exit 1
$I send ":login samuel"; $I cr; sleep 3
$I send ":dump"; $I cr; sleep 4
$I send "load links crdir sorry"; $I cr; wait_for "FILE=" 30 || exit 1
$I send "*;* *"; $I cr; wait_for "E-O-T" 300 || exit 1; sleep 2
$I send "quit"; $I cr; sleep 4
$I send ":listf oshrdl;"; $I cr; sleep 6
$I cap 15
$I stop
echo RESTAURADO
