# Enumera os atomos da imagem: cabecalho com LH=777777 e RH=lista de propriedades com PNAME.
# Exclui o espaco de palavras inteiras (FWS) e de flonums, onde LH=777777 e so um numero negativo.
import os, pickle, sys
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from mem import chars, LH, RH
mem = pickle.load(open(os.path.join(HERE, 'moby.pkl'), 'rb'))
PNAME = 0o21731
def car(p): return LH(mem.get(p, 0))
def cdr(p): return RH(mem.get(p, 0))
atoms = {}
for a, v in mem.items():
    if LH(v) == 0o777777 and a >= 0o16220 and not (0o215330 <= a < 0o226747):
        l = RH(v); n = 0; nm = None
        while l and n < 300:
            if car(l) == PNAME:
                pn = car(cdr(l)); s = ''; k = 0
                while pn and k < 60: s += chars(mem.get(car(pn), 0)); pn = cdr(pn); k += 1
                nm = s.rstrip('\0'); break
            l = cdr(cdr(l)); n += 1
        if nm: atoms[a] = nm
byname = {}
for a, nm in atoms.items(): byname.setdefault(nm, []).append(a)
pickle.dump((atoms, byname), open(os.path.join(HERE, 'atoms.pkl'), 'wb'))
print(len(atoms), 'atomos com nome')
