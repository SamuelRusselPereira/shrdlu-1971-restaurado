#!/bin/bash
# Sessao nova + todos os ajustes + definicoes de rastreio e SHRDLU2 (inertes); frases 1 a 12 do roteiro;
# depois um espaco, Ctrl-G e :pdump OSHRDL;S12 DUMP (estado logo antes da frase 13).
T=/home/claude/work/its_tape; I=$T/its.sh
cd $T
echo "=== sessao ponto de controle S12 $(date -u)" >> console.log
EXTRAS="$T/smone_corrigida.lisp $T/smprop_corrigida.lisp $T/trace_defs.lisp $T/trace_lista.lisp $T/shrdlu2.lisp" FINDQ_DEF="$(cat $T/findq_reconstruida.lisp)" bash $T/exp_findq_boot.sh > $T/ponto12_boot.txt 2>&1
echo "=== ponto de controle: frases 1-12" > $T/dialogo_ponto12.txt
ATE=12 python3 -I $T/dialogo2.py 1 $T/dialogo_ponto12.txt > $T/dialogo_ponto12.log 2>&1
sleep 2; $I send " "; sleep 5; $I ctrl g; sleep 3
$I ctrl z; sleep 2; $I send ":pdump oshrdl;s12 dump"; $I cr; sleep 8
$I esc; sleep 0.5; $I send "p"; sleep 3
echo FIM >> $T/dialogo_ponto12.log
