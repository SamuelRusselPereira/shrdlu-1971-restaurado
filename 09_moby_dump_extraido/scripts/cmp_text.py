# Compara o texto de cada definicao interpretada dos arquivos Z com a definicao achada no moby dump.
import os
import sys, glob, os, difflib, pickle
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import extract as X
from mlread import read_all, show, Sym
THEOREM = X.byname['THEOREM'][0]
def moby_def(nm, kind):
    for at in X.byname.get(nm, []):
        for ind, val in X.props(at):
            if (kind == 'THEOREM' and ind == THEOREM) or (kind != 'THEOREM' and ind == X.A.get(kind)):
                return X.pp(val)
    return None
rows = []
for f in sorted(glob.glob(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '07_its_vault_SHRDLU_original', 'z.*'))):
    base = os.path.basename(f)
    for form in read_all(open(f, encoding='latin-1').read()):
        if not isinstance(form, tuple): continue
        lst, dot = form
        if not lst or not isinstance(lst[0], Sym): continue
        head = lst[0]
        if head == 'DEFUN' and len(lst) >= 3:
            nm = lst[1]; kind = 'EXPR'; rest = lst[2:]
            if isinstance(rest[0], Sym) and rest[0] in ('FEXPR', 'EXPR', 'MACRO'): kind = str(rest[0]); rest = rest[1:]
            args = rest[0]; body = rest[1:]
            lam = show(([Sym('LAMBDA'), args] + body, None))
        elif head == 'DEFPROP' and len(lst) == 4:
            nm, lamx, kind = lst[1], lst[2], str(lst[3]); lam = show(lamx)
        elif head == 'DEFTHEOREM' and len(lst) == 3:
            nm, kind, lam = lst[1], 'THEOREM', show(lst[2])
        else: continue
        m = moby_def(nm, kind)
        if m is None: st = 'nao-interpretada'
        elif m == lam: st = 'IDENTICA'
        else: st = 'DIFERENTE'
        rows.append((base, str(nm), kind, st, lam, m))
pickle.dump(rows, open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'cmp_rows.pkl'), 'wb'))
from collections import Counter
print(Counter(r[3] for r in rows))
for b in sorted(set(r[0] for r in rows)):
    print(b, Counter(r[3] for r in rows if r[0] == b))
