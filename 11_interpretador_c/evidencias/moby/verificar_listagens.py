#!/usr/bin/env python3
"""F31: refaz as listagens de listagens_moby.txt com desmonta.py e compara linha a linha.
Uso: MOBY_PKL=.../moby.pkl ATOMS_PKL=.../atoms.pkl python3 verificar_listagens.py
Para gerar moby.pkl e atoms.pkl a partir de moby.dump (SHA-256 6d962b97...cb0ef0a3):
  dis10 -Wits -Snone moby.dump > moby_dis.txt      (github.com/larsbrinkhoff/pdp10-its-disassembler)
  python3 mem.py moby_dis.txt moby.pkl ; python3 atoms.py   (09_moby_dump_extraido/scripts do repositorio)"""
import re, subprocess, sys
from pathlib import Path
B = Path(__file__).resolve().parent
txt = (B/'listagens_moby.txt').read_text().splitlines()
secoes, atual = [], None
for l in txt:
    if l.startswith('=== '):
        m = re.match(r'^=== (\S+) \(a partir de ([0-7]+)\)$', l)
        atual = [m.group(1), m.group(2), []] if m else [l[4:40], None, []]   # None = secao de listas constantes
        secoes.append(atual)
    elif atual is not None and l.strip(): atual[2].append(l)
ok = True
for nome, end, linhas in secoes:
    if end is None:
        args = ['--lista'] + [x.split()[0] for x in linhas]
    else:
        args = [end, str(len(linhas))]
    r = subprocess.run([sys.executable, str(B/'desmonta.py'), *args], capture_output=True, text=True, check=True)
    igual = r.stdout.splitlines() == linhas
    ok &= igual
    print(f'{nome:36s} {end or "listas":>6s} {len(linhas):4d} linhas', 'IGUAL' if igual else 'DIFERENTE')
# enderecos de entrada (propriedades SUBR/FSUBR dos atomos)
e = (B/'enderecos_de_entrada.txt').read_text().splitlines()
r = subprocess.run([sys.executable, str(B/'desmonta.py'), '--props', *[x.split()[0] for x in e]], capture_output=True, text=True, check=True)
igual = r.stdout.splitlines() == e; ok &= igual
print(f'{"enderecos_de_entrada.txt":36s} {"props":>6s} {len(e):4d} linhas', 'IGUAL' if igual else 'DIFERENTE')
print('LISTAGENS CONFEREM' if ok else 'LISTAGENS NAO CONFEREM'); sys.exit(0 if ok else 1)
