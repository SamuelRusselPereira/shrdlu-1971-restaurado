#!/bin/bash
# Rodada 15: sessao nova + todos os ajustes (ver COMO_RODAR.md, passo 6); as 46 frases.
T=/home/claude/work/its_tape
cd $T
echo "=== sessao rodada 15 completa $(date -u)" >> console.log
EXTRAS="$T/smone_corrigida.lisp $T/smprop_corrigida.lisp $T/smadj_corrigida.lisp $T/modal_zerado.lisp $T/goalize2_corrigida.lisp $T/remendo_ng.lisp $T/smit3_corrigida.lisp $T/who_corrigida.lisp $T/loc_thmlist_corrigida.lisp $T/smrsq_corrigida.lisp $T/smng1_corrigida.lisp $T/size_corrigida.lisp" FINDQ_DEF="$(cat $T/findq_reconstruida.lisp)" bash $T/exp_findq_boot.sh > $T/rodada15c_boot.txt 2>&1
echo "=== sessao nova: SMNGOF Z + VALUE NIL + READCH + FINDQ reconstruida + SMONE, SMPROP e SMADJ corrigidas + MODAL zerado + GOALIZE2 (negacao por ultimo) + remendos da gramatica (PRONREL -> REL) + SMIT3 e WHO corrigidas + THMLIST de #LOC + SMRSQ (BUT), SMNG1 (anything) e SIZE (mao); todas as frases" > $T/dialogo_moby_v17.txt
python3 -I $T/dialogo2.py 1 $T/dialogo_moby_v17.txt > $T/dialogo_run17.log 2>&1
echo FIM >> $T/dialogo_run17.log
