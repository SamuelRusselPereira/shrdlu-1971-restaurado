#!/bin/bash
# SHRDLU de 1971 no interpretador em C. Digite as frases em MAIUSCULAS depois de READY/ (como no ITS).
# Sair: Ctrl-C. Compila na primeira vez (precisa de um compilador C: cc, gcc ou clang).
# SHRDLU (1971) on the C interpreter. Type sentences in UPPER CASE after READY/. Quit: Ctrl-C.
D=$(cd "$(dirname "$0")" && pwd)
cd "$D" || exit 1
M=${MACLISP:-build/maclisp}
if [ ! -x "$M" ]; then
  mkdir -p build && ${CC:-cc} -std=c99 -O2 -o "$M" maclisp.c -lm || exit 1
fi
exec "$M" $(grep -v '^#' carga.txt) -e '(SHRDLU)'
