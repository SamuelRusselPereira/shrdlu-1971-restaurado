#!/bin/bash
# Rodada 11: sessao nova + todos os ajustes (SMNGOF Z, VALUE NIL, READCH, FINDQ, SMONE e SMPROP corrigidas); as 46 frases.
T=/home/claude/work/its_tape
cd $T
echo "=== sessao rodada 11 completa $(date -u)" >> console.log
EXTRAS="$T/smone_corrigida.lisp $T/smprop_corrigida.lisp $T/smadj_corrigida.lisp $T/modal_zerado.lisp $T/goalize2_corrigida.lisp" FINDQ_DEF="$(cat $T/findq_reconstruida.lisp)" bash $T/exp_findq_boot.sh > $T/rodada11c_boot.txt 2>&1
echo "=== sessao nova: SMNGOF Z + VALUE NIL + READCH + FINDQ reconstruida + SMONE, SMPROP e SMADJ corrigidas + MODAL zerado + GOALIZE2 (negacao por ultimo); todas as frases" > $T/dialogo_moby_v13.txt
python3 -I $T/dialogo2.py 1 $T/dialogo_moby_v13.txt > $T/dialogo_run13.log 2>&1
echo FIM >> $T/dialogo_run13.log
