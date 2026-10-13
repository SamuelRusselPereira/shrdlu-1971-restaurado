# Para cada definicao de topo dos arquivos Z, verifica como ela aparece no moby dump.
import os
import re, sys, pickle, glob, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import extract as X
Z = sorted(glob.glob(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '7_its_vault_SHRDLU_original', 'z.*')))
defs = []
for f in Z:
    txt = open(f, encoding='latin-1').read()
    for m in re.finditer(r'^\((DEFUN|DEFPROP|DEFTHEOREM|PDEFINE)\s*([^\s()]+)', txt, re.M):
        defs.append((os.path.basename(f), m.group(1), m.group(2)))
kinds = {v: k for k, v in X.A.items()}
THEOREM = X.byname.get('THEOREM', [None])[0]
res = {}
for f, form, nm in defs:
    a = X.byname.get(nm)
    if not a: st = 'AUSENTE'
    else:
        inds = set()
        for at in a:
            for ind, val in X.props(at):
                if ind in kinds: inds.add(kinds[ind])
                if ind == THEOREM: inds.add('THEOREM')
        st = '+'.join(sorted(inds)) if inds else 'so-nome'
    res.setdefault(f, []).append((form, nm, st))
tot = {}
for f in res:
    c = {}
    for form, nm, st in res[f]: c[st] = c.get(st, 0) + 1; tot[st] = tot.get(st, 0) + 1
    print(f, len(res[f]), c)
print('TOTAL', sum(tot.values()), tot)
pickle.dump(res, open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'zdefs.pkl'), 'wb'))
