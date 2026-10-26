#!/bin/bash
# F31: sessao nova do MOBY (sem ajustes de 2026), teste minimo do THFAIL no nivel do Lisp, antes de (SHRDLU).
T=/home/claude/work/its_tape; I=$T/its.sh; OUT=$1
bash $T/f31/boot_moby_f31.sh > /dev/null 2>&1 || { echo BOOT-FALHOU; exit 1; }
M=$(stat -c %s $T/console.log)
while read -r l; do [ -n "$l" ] && { $I send "$l"; sleep 4; }; done < $T/f31/teste_minimo_thfail.lisp
sleep 6
tail -c +$((M+1)) $T/console.log | tr -d '\r' > $OUT
$I stop
