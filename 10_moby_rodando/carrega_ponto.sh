#!/bin/bash
# Recarrega um ponto de controle (OSHRDL;<nome> DUMP) num job novo. Uso: carrega_ponto.sh s41 [arquivos .lisp para digitar]
T=/home/claude/work/its_tape; I=$T/its.sh; P=$1; shift
$I ctrl g; sleep 2; $I ctrl z; sleep 2; $I send ":kill"; $I cr; sleep 3
$I send ":job $P"; $I cr; sleep 2; $I send ":load oshrdl;$P dump"; $I cr; sleep 6; $I esc; sleep 0.5; $I send "g"; sleep 4
for f in "$@"; do while read -r l; do [ -n "$l" ] && { $I send "$l"; sleep 4; }; done < $f; done
