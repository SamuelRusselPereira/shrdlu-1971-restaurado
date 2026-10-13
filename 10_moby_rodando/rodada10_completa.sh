#!/bin/bash
# Rodada 10: sessao nova + todos os ajustes (SMNGOF Z, VALUE NIL, READCH, FINDQ, SMONE e SMPROP corrigidas); as 46 frases.
T=/home/claude/work/its_tape
cd $T
echo "=== sessao rodada 10 completa $(date -u)" >> console.log
EXTRAS="$T/smone_corrigida.lisp $T/smprop_corrigida.lisp" FINDQ_DEF="$(cat $T/findq_reconstruida.lisp)" bash $T/exp_findq_boot.sh > $T/rodada10c_boot.txt 2>&1
echo "=== sessao nova: SMNGOF Z + VALUE NIL + READCH + FINDQ reconstruida + SMONE e SMPROP corrigidas; todas as frases" > $T/dialogo_moby_v12.txt
python3 -I $T/dialogo2.py 1 $T/dialogo_moby_v12.txt > $T/dialogo_run12.log 2>&1
echo FIM >> $T/dialogo_run12.log
