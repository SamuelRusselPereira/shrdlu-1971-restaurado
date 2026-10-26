#!/usr/bin/env python3
"""Compara saidas do interpretador em C com os logs brutos do SHRDLU original no ITS (evidencias/its/).
Compares C-interpreter outputs with the raw logs of the original SHRDLU on ITS (evidencias/its/).

  sonda ITS C    : rastro da frase 16 (de F30-INICIO-MOBY ate F31-RESULTADO), de dois jeitos:
                   (1) sem nenhum espaco em branco; (2) linha a linha, juntando cada linha de 72 caracteres
                   do ITS com a seguinte (o MacLisp do ITS quebra as linhas na coluna 72, ate no meio de um atomo).
  extras ITS C   : respostas a frases fora do dialogo da tese.
  minimo ITS C   : resultado do teste minimo do THFAIL (linha F31-RESULTADO).
"""
import re, sys
from pathlib import Path


def _texto(caminho):
    return Path(caminho).read_text(encoding='latin-1').replace('\r', '')


def trecho_sonda(caminho):
    t = _texto(caminho)
    i = t.index('F30-INICIO-MOBY'); j = t.index('(F31-RESULTADO', i); k = t.find('\n', j)
    return t[i:k if k >= 0 else len(t)]


def compara_sonda(its, c):
    a, b = trecho_sonda(its), trecho_sonda(c)
    sem_espaco = re.sub(r'\s+', '', a) == re.sub(r'\s+', '', b)
    def linhas(s, largura=None):
        out, acc = [], ''
        for l in s.split('\n'):
            acc += l
            if largura and len(l) == largura:
                continue
            out.append(acc.rstrip()); acc = ''
        if acc:
            out.append(acc.rstrip())
        return [l for l in out if l.strip()]
    la, lb = linhas(a, 72), linhas(b)
    return {'eventos_its': a.count('(F31-'), 'eventos_c': b.count('(F31-'), 'igual_sem_espacos': sem_espaco,
            'igual_linha_a_linha': la == lb, 'linhas_its': len(la), 'linhas_c': len(lb)}


def respostas_its(caminho):
    """Respostas de uma transcricao do dialogo2.py no ITS (blocos '=== n frase')."""
    t = _texto(caminho)
    if 'F30-INICIO-MOBY' in t:                        # tira a saida da sonda, se houver
        i = t.index('F30-INICIO-MOBY'); j = t.index('(F31-RESULTADO', i); k = t.index('\n', j)
        t = t[:i] + t[k:]
    t = re.sub(r'^\(F31-ANSTHM [^\n]*\n', '', t, flags=re.M)
    blocos = re.split(r'^=== (\d+) ([^\n]*)\n', t, flags=re.M)[1:]
    out = []
    for n, frase, b in zip(blocos[0::3], blocos[1::3], blocos[2::3]):
        ll = b.splitlines()
        k = next(i for i, x in enumerate(ll) if x.startswith('*'))
        r = ' '.join(x for x in ll[k:] if x.strip() and not x.startswith('READY/')).replace('*', ' ')
        out.append((int(n), frase.strip(), ' '.join(r.split())))
    return out


def respostas_c(caminho, frases):
    t = _texto(caminho).replace('\r', '\n')
    out = []
    for b, frase in zip(t.split('READY/')[1:len(frases) + 1], frases):
        ll = [x.strip() for x in b.splitlines() if x.strip()]
        if ll and ' '.join(ll[0].split()).upper() == ' '.join(frase.split()).upper():
            ll = ll[1:]
        out.append(' '.join(' '.join(x for x in ll if x != '*').replace('*', ' ').split()))
    return out


def compara_extras(its, c):
    ri = respostas_its(its)
    rc = respostas_c(c, [f for _, f, _ in ri])
    linhas = [(n, f, a, b, a == b) for (n, f, a), b in zip(ri, rc)]
    return {'frases': len(ri), 'iguais': sum(1 for x in linhas if x[4]), 'linhas': linhas}


def resultado_minimo(caminho):
    m = re.findall(r'^\(F31-RESULTADO (.*)\)\s*$', _texto(caminho), re.M)
    return m[-1].strip() if m else None


if __name__ == '__main__':
    modo, its, c = sys.argv[1:4]
    if modo == 'sonda':
        print(compara_sonda(its, c))
    elif modo == 'extras':
        r = compara_extras(its, c)
        for n, f, a, b, ok in r['linhas']:
            print(n, 'IGUAL' if ok else 'DIFERENTE', '|', f, '->', a, '' if ok else f'| C: {b}')
        print(f"{r['iguais']} de {r['frases']} iguais ao ITS")
    elif modo == 'minimo':
        print('ITS:', resultado_minimo(its), '| C:', resultado_minimo(c))
