#!/bin/bash
# Rodada 9: sessao nova + ajustes da rodada 8 + FINDQ reconstruida; as 46 frases.
T=/home/claude/work/its_tape
cd $T
echo "=== sessao rodada 9 $(date -u)" >> console.log
FINDQ_DEF="$(cat $T/findq_reconstruida.lisp)" bash $T/exp_findq_boot.sh > $T/rodada9_boot.txt 2>&1
echo "=== sessao nova: SMNGOF da fonte Z + VALUE NIL + READCH (so digitos) + FINDQ reconstruida; todas as frases" > $T/dialogo_moby_v9.txt
python3 -I $T/dialogo2.py 1 $T/dialogo_moby_v9.txt > $T/dialogo_run9.log 2>&1
echo FIM >> $T/dialogo_run9.log
