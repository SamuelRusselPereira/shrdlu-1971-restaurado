# Compara uma transcricao do moby dump com o dialogo da tese, frase a frase, PELA POSICAO
# (a frase "why did you do that?" aparece tres vezes, entao nao da para casar pelo texto).
# Uso: python3 compara.py dialogo_moby_v9.txt > tabela_v9.tsv
# Marca automatica: '=' igual, '=+' igual com texto a mais, '?' diferente (revisar a mao).
import re, sys
tese = []
for l in open('/home/claude/work/kit/dialogo_1971.tsv', encoding='utf-8'):
    if l.startswith('#') or not l.strip(): continue
    p = l.rstrip('\n').split('\t')
    tese.append((p[0], p[2].strip(), p[3].strip()))
txt = open(sys.argv[1], encoding='latin-1').read()
blocks = re.split(r'^=== (\d+) (.*)$', txt, flags=re.M)
def norm(s): return re.sub(r'[^A-Z0-9]+', ' ', s.upper()).strip()
for k in range(1, len(blocks) - 2, 3):
    i, frase, corpo = int(blocks[k]), blocks[k+1].strip(), blocks[k+2]
    linhas = [x.rstrip() for x in corpo.split('\n')]
    j = next((j for j, x in enumerate(linhas) if x.startswith('*')), len(linhas))
    resp = ' '.join(x.strip() for x in linhas[j:] if x.strip() and not x.strip().startswith('READY') and x.strip() != '*')
    resp = re.sub(r'\s+', ' ', resp.replace('*', ' ')).strip()
    n, hum, t = tese[i - 1]
    assert hum.lower() == frase.lower(), (i, hum, frase)
    nt, nr = norm(t), norm(resp)
    v = '=' if nr == nt else ('=+' if nt and (' ' + nt + ' ') in (' ' + nr + ' ') else '?')
    print('\t'.join((str(i), n, frase, t, resp, v)))
