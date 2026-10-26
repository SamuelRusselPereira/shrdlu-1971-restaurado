#!/bin/bash
# F31: sessao nova do MOBY com a configuracao exata da rodada 16 (rodada16_completa.sh) + sonda F31; frases 1 a 16.
T=/home/claude/work/its_tape; I=$T/its.sh; OUT=$1
$I stop 2>/dev/null; sleep 1
bash $T/f31/boot_moby_f31.sh || { echo BOOT-FALHOU; exit 1; }
$I send "$(cat $T/smngof_z.lisp)"; sleep 3
while read -r l; do [ -n "$l" ] && { $I send "$l"; sleep 3; }; done < $T/fixes_ambiente.lisp
$I send "$(cat $T/findq_reconstruida.lisp)"; sleep 3
EXTRAS="$T/smone_corrigida.lisp $T/smprop_corrigida.lisp $T/smadj_corrigida.lisp $T/modal_zerado.lisp $T/goalize2_corrigida.lisp $T/remendo_ng.lisp $T/smit3_corrigida.lisp $T/who_corrigida.lisp $T/loc_thmlist_corrigida.lisp $T/smrsq_corrigida.lisp $T/smng1_corrigida.lisp $T/size_corrigida.lisp $T/ans2_corrigida.lisp $T/f31/sonda_turno16.lisp"
for f in $EXTRAS; do while read -r l; do [ -n "$l" ] && { $I send "$l"; sleep 3; }; done < $f; done
$I send "(SHRDLU)"; sleep 6
echo "=== F31: sessao nova, configuracao da rodada 16 + sonda F31 (turno 16) $(date -u)" > $OUT
ATE=16 python3 -I $T/dialogo2.py 1 $OUT > $T/f31/dialogo_ate16.log 2>&1
sleep 2
$I stop
echo SESSAO-FIM
