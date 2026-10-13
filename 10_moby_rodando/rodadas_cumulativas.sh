#!/bin/bash
# Tabela cumulativa: tres rodadas com subconjuntos dos ajustes da rodada 16 (sessao nova em cada uma, as 46 frases).
T=/home/claude/work/its_tape
cd $T
REST="$T/smone_corrigida.lisp $T/smprop_corrigida.lisp $T/smadj_corrigida.lisp $T/smit3_corrigida.lisp $T/loc_thmlist_corrigida.lisp $T/ans2_restauracao.lisp"
run() { # nome, FINDQ_DEF, EXTRAS
  echo "=== rodada cumulativa $1 $(date -u)" >> console.log
  FINDQ_DEF="$2" EXTRAS="$3" bash $T/cum_boot.sh > $T/cum_$1_boot.txt 2>&1
  echo "=== rodada cumulativa $1: $4" > $T/dialogo_cum_$1.txt
  python3 -I $T/dialogo2.py 1 $T/dialogo_cum_$1.txt > $T/dialogo_cum_$1.log 2>&1
  echo "FIM $1 $(date -u)" >> $T/dialogo_cum_$1.log
}
run A "" "" "4 variaveis + READCH + SMNGOF dos arquivos Z (original + ambiente)"
run B "$(cat $T/findq_reconstruida.lisp)" "" "A + FINDQ reconstruida"
run C "$(cat $T/findq_reconstruida.lisp)" "$REST" "B + 6 restauracoes (SMONE SMPROP SMADJ SMIT3 THMLIST-#LOC ANS2-parte1)"
echo "TUDO-FIM $(date -u)" >> $T/cum_status.txt
