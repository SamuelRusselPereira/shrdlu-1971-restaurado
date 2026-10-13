# Extrai da imagem (moby dump) as definicoes interpretadas (EXPR/FEXPR/MACRO) como texto MacLisp,
import os
# e lista as funcoes compiladas (SUBR/FSUBR/LSUBR). Numeros impressos em octal, como no MacLisp (base 8).
import pickle, sys, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from mem import chars, LH, RH
mem = pickle.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'moby.pkl'), 'rb'))
atoms, byname = pickle.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'atoms.pkl'), 'rb'))
A = {k: byname[k][0] for k in ['PNAME','EXPR','FEXPR','SUBR','FSUBR','LSUBR','MACRO','VALUE','FIXNUM','FLONUM','QUOTE']}
INUM_LIM = 0o16220                 # 1..16217: numero imediato (valor = ponteiro - 1)
FWS_LO, FWS_HI = 0o215330, 0o226737 # espaco de palavras inteiras (NSPL..FLSL): fixnums e pnames
FLS_LO, FLS_HI = 0o226737, 0o226747 # espaco de flonums
def car(p): return LH(mem.get(p, 0))
def cdr(p): return RH(mem.get(p, 0))
def is_inum(p): return 0 < p < INUM_LIM
def is_fws(p): return FWS_LO <= p < FWS_HI
def is_fls(p): return FLS_LO <= p < FLS_HI
def is_atom(p): return p != 0 and not is_inum(p) and not is_fws(p) and not is_fls(p) and LH(mem.get(p, 0)) == 0o777777
def props(p):
    l = RH(mem[p]); out = []; n = 0
    while l and n < 300:
        out.append((car(l), car(cdr(l)))); l = cdr(cdr(l)); n += 1
    return out
names = {}
def name_of(p):
    if p in names: return names[p]
    nm = None
    for ind, val in props(p):
        if ind == A['PNAME']:
            s = ''; pn = val; k = 0
            while pn and k < 60: s += chars(mem.get(car(pn), 0)); pn = cdr(pn); k += 1
            nm = s.rstrip('\0'); break
    names[p] = nm; return nm
SPECIAL = set(" ()'/;.\"\t\n|")
def esc(s):
    return ''.join('/' + c if c in SPECIAL else c for c in s)
def num_atom(p):
    for ind, val in props(p):
        if ind == A['FIXNUM']:
            w = mem.get(val, 0); w = w - (1 << 36) if w >> 35 else w
            return ('-' if w < 0 else '') + oct(abs(w))[2:]
        if ind == A['FLONUM']:
            return '#<flonum %012o>' % mem.get(val, 0)
    return None
def pp(p, depth=0, seen=None):
    if p == 0: return 'NIL'
    if is_inum(p): return oct(p - 1)[2:]
    if is_fws(p):
        w = mem.get(p, 0); w = w - (1 << 36) if w >> 35 else w
        return ('-' if w < 0 else '') + oct(abs(w))[2:]
    if is_fls(p): return '#<flonum %012o>' % mem.get(p, 0)
    if p > 0o267777: return '#<?%o>' % p
    if is_atom(p):
        nm = name_of(p)
        if nm is not None: return esc(nm)
        na = num_atom(p)
        return na if na is not None else '#<atomo %o>' % p
    if depth > 60: return '#<fundo>'
    parts = []; n = 0
    while True:
        if n > 2000: parts.append('...'); break
        parts.append(pp(car(p), depth + 1)); p = cdr(p); n += 1
        if p == 0: break
        if is_inum(p) or is_fws(p) or is_fls(p) or is_atom(p) or p > 0o267777:
            parts.append('.'); parts.append(pp(p, depth + 1)); break
    if len(parts) == 2 and parts[0] == 'QUOTE': return "'" + parts[1]
    return '(' + ' '.join(parts) + ')'
if __name__ == '__main__':
    out = open(sys.argv[1], 'w'); stats = {}
    seen = set()
    for a in sorted(atoms):
        nm = atoms[a]
        if nm in seen: continue
        for ind, val in props(a):
            for kind in ['EXPR', 'FEXPR', 'MACRO']:
                if ind == A[kind]:
                    seen.add(nm); stats[kind] = stats.get(kind, 0) + 1
                    out.write('(DEFPROP %s\n %s\n %s)\n\n' % (esc(nm), pp(val), kind))
            for kind in ['SUBR', 'FSUBR', 'LSUBR']:
                if ind == A[kind]:
                    stats[kind] = stats.get(kind, 0) + 1
    out.close(); print(stats)
