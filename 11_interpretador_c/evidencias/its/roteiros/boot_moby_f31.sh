#!/bin/bash
# Liga o ITS no emulador (tmux), entra, e carrega o moby dump com as chaves do dialogo.
T=/home/claude/work/its_tape; I=$T/its.sh
wait_for() { for k in $(seq 1 ${2:-60}); do tail -c +$((M+1)) $T/console.log | tr '\r' '\n' | grep -q "$1" && return 0; sleep 1; done; echo "TIMEOUT esperando: $1"; return 1; }
M=$(stat -c %s $T/console.log); $I start; wait_for "DSKDMP" 90; sleep 8
# o DSKDMP as vezes perde as primeiras teclas (emulador frio): digita "its" ate ver o eco
for t in 1 2 3 4 5; do M2=$(stat -c %s $T/console.log); DELAY=0.4 $I send "its"; sleep 2; if tail -c +$((M2+1)) $T/console.log | tr -d "\r" | grep -q "its"; then break; fi; $I cr; sleep 3; done
$I cr; sleep 4
DELAY=0.3 $I send "styo+2/popj p,"; sleep 1; $I cr; sleep 2; $I esc; sleep 1; $I send "g"
wait_for "IN OPERATION" 90 || exit 1; sleep 3; $I ctrl z; wait_for "Happy hackin" 30 || exit 1
$I send ":login samuel"; $I cr; sleep 3
$I send ":job moby"; $I cr; sleep 2; $I send ":load oshrdl;moby dump"; $I cr; sleep 5; $I esc; $I send "g"; sleep 4
for f in "(SETQ NOSTOP T)" "(SETQ NOMEM NIL)" "(SETQ VALUE NIL)" "(SETQ PLANNERSEE ${PLANNERSEE:-NIL})"; do $I send "$f"; sleep 2; done
echo BOOT-OK
