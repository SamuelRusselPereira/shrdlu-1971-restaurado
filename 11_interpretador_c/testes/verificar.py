#!/usr/bin/env python3
"""Compara a transcricao das 46 frases com as duas referencias de testes/tabela_rodada16.tsv:
  - a rodada 16 do SHRDLU original no emulador do PDP-10 (ITS, 2026): coluna 5;
  - o dialogo impresso na tese (AI-TR-235, janeiro de 1971, secao 1.3): coluna 4.
Numeracao: a da tese (coluna 2), que pula o numero 38. Regra: so letras e numeros, em maiusculas
(a mesma das rodadas no emulador; nao e igualdade de bytes com o terminal).

Compares the 46-sentence transcript with the two references (ITS 2026 run, and the thesis printout).
Numbering is the thesis's own (it has no item 38). Rule: letters and digits only, upper case.

Uso / usage: python3 testes/verificar.py resultados/dialogo.txt
"""
import re, sys
from pathlib import Path

B = Path(__file__).resolve().parents[1]
TABELA = B / 'testes/tabela_rodada16.tsv'


def normal(s):
    return re.sub(r'[^A-Z0-9]+', ' ', s.upper()).strip()


def respostas(transcricao, frases):
    """Resposta de cada frase: o que vem depois do eco da frase ate o proximo READY/."""
    t = Path(transcricao).read_text(encoding='latin-1').replace('\r', '\n')
    blocos = t.split('READY/')[1:len(frases) + 1]
    out = []
    for b, frase in zip(blocos, frases):
        ll = [x.strip() for x in b.splitlines() if x.strip()]
        if ll and normal(ll[0]) == normal(frase):
            ll = ll[1:]                                # eco da frase digitada
        r = ' '.join(x for x in ll if x != '*' and not x.startswith('>>>')
                     and x not in ('LISTENING', 'NIL-AUTOMATICO'))
        out.append(' '.join(r.replace('*', ' ').split()))
    return out


def verificar(transcricao):
    linhas = [l.split('\t') for l in TABELA.read_text(encoding='utf-8').splitlines()
              if l.strip() and not l.startswith('#')]
    resp = respostas(transcricao, [l[2] for l in linhas])
    assert len(resp) == len(linhas), f'transcricao incompleta: {len(resp)} de {len(linhas)} frases'
    res = {'its_iguais': [], 'its_diferentes': [], 'tese_iguais': [], 'tese_com_texto_a_mais': [],
           'tese_diferentes': [], 'linhas': []}
    for (pos, num, frase, tese, its, _), c in zip(linhas, resp):
        num = int(num)
        (res['its_iguais'] if normal(c) == normal(its) else res['its_diferentes']).append(num)
        if normal(c) == normal(tese):
            res['tese_iguais'].append(num)
        elif normal(tese) in normal(c):
            res['tese_com_texto_a_mais'].append(num)
        else:
            res['tese_diferentes'].append(num)
        res['linhas'].append((num, frase, tese, its, c))
    return res


if __name__ == '__main__':
    r = verificar(sys.argv[1])
    for num, frase, tese, its, c in r['linhas']:
        marca = '=' if num in r['its_iguais'] else 'DIFERENTE DO ITS'
        print(f'{num:2d} {marca:16s} {frase}')
        if num in r['its_diferentes']:
            print(f'     ITS: {its}\n     C:   {c}')
    print()
    print(f"Rodada 16 do ITS (2026): {len(r['its_iguais'])} de {len(r['linhas'])} iguais"
          + (f"; diferentes: {r['its_diferentes']}" if r['its_diferentes'] else ''))
    print(f"Tese (jan. 1971): {len(r['tese_iguais'])} iguais, {len(r['tese_com_texto_a_mais'])} com texto a mais "
          f"{r['tese_com_texto_a_mais']}, {len(r['tese_diferentes'])} diferentes {r['tese_diferentes']}")
