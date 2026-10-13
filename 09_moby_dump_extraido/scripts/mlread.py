# Leitor minimo de MacLisp (base 8 por padrao; "N." = decimal; / escapa; ; comenta; ' = QUOTE)
import os
import re
class Sym(str): pass
def tokenize(txt):
    i, n = 0, len(txt); toks = []
    while i < n:
        c = txt[i]
        if c in ' \t\r\n\f\x03\x00': i += 1; continue
        if c == ';':
            while i < n and txt[i] != '\n': i += 1
            continue
        if c in "()'": toks.append(c); i += 1; continue
        s = ''; esc = False
        while i < n:
            c = txt[i]
            if c == '/' and i + 1 < n: s += txt[i+1]; i += 2; esc = True; continue
            if c in " \t\r\n\f()';\x03\x00": break
            s += c; i += 1
        toks.append(('ESC' if esc else 'ATOM', s))
    return toks
def atom(tok):
    kind, s = tok
    if kind == 'ATOM':
        if re.fullmatch(r'[+-]?[0-7]+', s): return int(s, 8)
        if re.fullmatch(r'[+-]?[0-9]+\.', s): return int(s[:-1], 10)
    return Sym(s)
def parse(toks, i=0):
    t = toks[i]
    if t == '(':
        lst = []; i += 1; dotted = None
        while toks[i] != ')':
            if toks[i] == ('ATOM', '.') :
                dotted, i = parse(toks, i + 1); continue
            x, i = parse(toks, i); lst.append(x)
        return (lst, dotted), i + 1
    if t == "'":
        x, i = parse(toks, i + 1); return ([Sym('QUOTE'), x], None), i
    if t == ')': raise ValueError('parentese extra')
    if isinstance(t, tuple) and t[0] == 'ATOM' and t[1].startswith('$?') and len(t[1]) > 2:
        return ([Sym('THV'), Sym(t[1][2:])], None), i + 1
    if isinstance(t, tuple) and t[0] == 'ATOM' and t[1].startswith('$_') and len(t[1]) > 2:
        return ([Sym('THNV'), Sym(t[1][2:])], None), i + 1
    if isinstance(t, tuple) and t[0] == 'ATOM' and t[1] == '$E':
        x, j = parse(toks, i + 1); return ([Sym('THEV'), x], None), j
    return atom(t), i + 1
SPECIAL = set(" ()'/;.\"\t\n|")
def esc(s): return ''.join('/' + c if c in SPECIAL else c for c in s)
def show(x):
    if isinstance(x, tuple):
        lst, dot = x
        if dot is None and len(lst) == 2 and lst[0] == 'QUOTE' and isinstance(lst[0], Sym): return "'" + show(lst[1])
        if not lst and dot is None: return 'NIL'
        body = ' '.join(show(e) for e in lst)
        if dot is not None and not (isinstance(dot, Sym) and dot == 'NIL'): body += ' . ' + show(dot)
        return '(' + body + ')'
    if isinstance(x, int): return ('-' if x < 0 else '') + oct(abs(x))[2:]
    return esc(x)
def read_all(txt):
    toks = tokenize(txt); i = 0; out = []
    while i < len(toks):
        try:
            x, i = parse(toks, i); out.append(x)
        except (IndexError, ValueError):
            break
    return out
