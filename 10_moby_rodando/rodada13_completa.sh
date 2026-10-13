#!/bin/bash
# Rodada 13: sessao nova + todos os ajustes (ver COMO_RODAR.md, passo 6); as 46 frases.
T=/home/claude/work/its_tape
cd $T
echo "=== sessao rodada 13 completa $(date -u)" >> console.log
EXTRAS="$T/smone_corrigida.lisp $T/smprop_corrigida.lisp $T/smadj_corrigida.lisp $T/modal_zerado.lisp $T/goalize2_corrigida.lisp $T/remendo_ng.lisp $T/smit3_corrigida.lisp $T/who_corrigida.lisp" FINDQ_DEF="$(cat $T/findq_reconstruida.lisp)" bash $T/exp_findq_boot.sh > $T/rodada13c_boot.txt 2>&1
echo "=== sessao nova: SMNGOF Z + VALUE NIL + READCH + FINDQ reconstruida + SMONE, SMPROP e SMADJ corrigidas + MODAL zerado + GOALIZE2 (negacao por ultimo) + remendos da gramatica (PRONREL -> REL) + SMIT3 e WHO corrigidas; todas as frases" > $T/dialogo_moby_v15.txt
python3 -I $T/dialogo2.py 1 $T/dialogo_moby_v15.txt > $T/dialogo_run15.log 2>&1
echo FIM >> $T/dialogo_run15.log
