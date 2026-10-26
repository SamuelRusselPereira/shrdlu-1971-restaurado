#!/usr/bin/env python3
"""Teste completo do interpretador / full test of the interpreter.

  python3 testar.py

1. Confere o SHA-256 de cada arquivo listado em SHA256SUMS (resultados/ fica de fora: este teste o refaz).
2. Compila maclisp.c com GCC em C99 estrito (-Wall -Wextra -Wpedantic -Werror) e, se houver Clang,
   tambem com o UndefinedBehaviorSanitizer (UBSan).
3. Para cada compilador:
   a. as 46 frases da tese: tem de dar 46 de 46 iguais a rodada 16 do ITS, e contra a tese
      36 iguais, 5 com texto a mais (3, 7, 8, 14, 26) e 5 diferentes (2, 19, 24, 37, 46);
   b. controle: sem ajustes/imagem/planner_como_na_imagem.lisp, a frase 16 tem de divergir (45 de 46);
   c. teste minimo do THFAIL: tem de dar o mesmo resultado do ITS, (#FFF 7);
   d. frases 1 a 16 com a sonda: o rastro da frase 16 tem de ser igual ao do ITS, sem espacos e linha a linha;
   e. as 10 frases fora do dialogo da tese: as 10 respostas tem de ser iguais as do ITS.
4. shrdlu.sh (o modo interativo) tem de abrir o SHRDLU num terminal e responder a uma frase digitada.
5. GCC e Clang tem de produzir o mesmo dialogo, byte a byte.
Grava resultados/RESULTADOS.json. Precisa de Linux ou macOS, Python 3 e GCC (Clang e opcional).
"""
import hashlib, json, os, shutil, subprocess, sys
from pathlib import Path

B = Path(__file__).resolve().parent
sys.path.insert(0, str(B / 'testes'))
from verificar import verificar                                   # noqa: E402
from comparar_its import compara_sonda, compara_extras, resultado_minimo   # noqa: E402
from dialogo import carga                                         # noqa: E402

H = lambda b: hashlib.sha256(b).hexdigest()
RES = B / 'resultados'; RES.mkdir(exist_ok=True)
BUILD = B / 'build'; BUILD.mkdir(exist_ok=True)
ITS = B / 'evidencias/its'
PLANNER = 'ajustes/imagem/planner_como_na_imagem.lisp'
falhas = []


def confere(cond, msg):
    print(('  ok   ' if cond else '  FALHOU ') + msg, flush=True)
    if not cond:
        falhas.append(msg)


# 1. integridade
somas = B / 'SHA256SUMS'
if somas.exists():
    n = ruins = 0
    for linha in somas.read_text().splitlines():
        if not linha.strip():
            continue
        sha, nome = linha.split(None, 1)
        nome = nome.lstrip('*').strip()
        if nome.startswith('resultados/'):
            continue
        n += 1
        if not (B / nome).is_file() or H((B / nome).read_bytes()) != sha:
            ruins += 1; print('  arquivo alterado ou ausente:', nome)
    confere(ruins == 0, f'SHA-256 de {n} arquivos (SHA256SUMS)')

# 2. compilacao
compiladores = []
if shutil.which('gcc'):
    compiladores.append(('gcc', ['gcc', '-std=c99', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-Werror']))
if shutil.which('clang'):
    compiladores.append(('clang_ubsan', ['clang', '-std=c99', '-O1', '-g', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                                         '-fsanitize=undefined', '-fno-sanitize-recover=all']))
confere(bool(compiladores), 'ha um compilador C (gcc ou clang)')
resumo = {}


def dialogo(binario, saida, *extra):
    r = subprocess.run([sys.executable, str(B / 'testes/dialogo.py'), '--maclisp', str(binario),
                        '--saida', str(saida), *extra], capture_output=True, text=True, timeout=900)
    (Path(str(saida) + '.log')).write_text(r.stdout + r.stderr)
    texto = Path(saida).read_text(encoding='latin-1')
    confere('runtime error' not in texto, f'{Path(saida).name}: sem erro do sanitizador')
    return r.returncode == 0


for nome, cc in compiladores:
    print(f'\n== {nome}', flush=True)
    binario = BUILD / f'maclisp-{nome}'
    r = subprocess.run(cc + [str(B / 'maclisp.c'), '-lm', '-o', str(binario)], capture_output=True, text=True)
    confere(r.returncode == 0, f'compila sem nenhum aviso ({" ".join(cc[1:])})')
    if r.returncode != 0:
        print(r.stderr); continue
    res = {}
    # a. 46 frases
    d46 = RES / f'{nome}_dialogo_46.txt'
    confere(dialogo(binario, d46), '46 frases chegaram a READY/')
    v = verificar(d46)
    confere(len(v['its_iguais']) == 46, f"rodada 16 do ITS: {len(v['its_iguais'])} de 46 iguais")
    confere((len(v['tese_iguais']), v['tese_com_texto_a_mais'], v['tese_diferentes']) ==
            (36, [3, 7, 8, 14, 26], [2, 19, 24, 37, 46]),
            f"tese: {len(v['tese_iguais'])} iguais, texto a mais {v['tese_com_texto_a_mais']}, diferentes {v['tese_diferentes']}")
    bruto = d46.read_bytes()
    res['dialogo_46'] = {'its_iguais': len(v['its_iguais']), 'tese_iguais': len(v['tese_iguais']),
                         'tese_com_texto_a_mais': v['tese_com_texto_a_mais'], 'tese_diferentes': v['tese_diferentes'],
                         'sha256_dialogo': H(bruto[bruto.index(b'READY/'):])}
    # b. controle sem o Micro-Planner da imagem
    ctl = RES / f'{nome}_controle_sem_planner_da_imagem.txt'
    dialogo(binario, ctl, '--sem', PLANNER)
    vc = verificar(ctl)
    confere(vc['its_diferentes'] == [16], f"controle sem o THFAIL da imagem: diverge em {vc['its_diferentes']} (esperado [16])")
    res['controle_sem_planner_da_imagem'] = {'its_diferentes': vc['its_diferentes']}
    # c. teste minimo do THFAIL (sem dialogo: so carrega e avalia o arquivo)
    arqs = carga() + ['testes/teste_minimo_thfail.lisp']
    r = subprocess.run([str(binario), *arqs], capture_output=True, text=True, errors='replace',
                       stdin=subprocess.DEVNULL, timeout=300, cwd=B)
    (RES / f'{nome}_teste_minimo.txt').write_text(r.stdout)
    c_min, its_min = resultado_minimo(RES / f'{nome}_teste_minimo.txt'), resultado_minimo(ITS / 'teste_minimo_ITS.txt')
    confere(c_min == its_min == '(#FFF 7)', f'teste minimo do THFAIL: C {c_min}, ITS {its_min}')
    res['teste_minimo'] = c_min
    # d. sonda da frase 16
    son = RES / f'{nome}_sonda_frase16.txt'
    dialogo(binario, son, '--max', '16', '--extra', str(B / 'testes/sonda_turno16.lisp'))
    cs = compara_sonda(ITS / 'sessao_turno16_ITS.txt', son)
    confere(cs['igual_sem_espacos'] and cs['igual_linha_a_linha'],
            f"sonda da frase 16: {cs['eventos_c']} eventos no C e {cs['eventos_its']} no ITS; "
            f"iguais sem espacos: {cs['igual_sem_espacos']}; iguais linha a linha ({cs['linhas_c']} linhas): {cs['igual_linha_a_linha']}")
    res['sonda_frase16'] = cs
    # e. frases fora do dialogo
    ext = RES / f'{nome}_frases_extras.txt'
    dialogo(binario, ext, '--frases', str(B / 'testes/frases_extras.txt'))
    ce = compara_extras(ITS / 'sessao_extras_ITS.txt', ext)
    confere(ce['iguais'] == ce['frases'] == 10, f"frases fora do dialogo: {ce['iguais']} de {ce['frases']} iguais ao ITS")
    res['frases_extras'] = {'iguais': ce['iguais'], 'frases': ce['frases']}
    resumo[nome] = res

# 4. modo interativo: shrdlu.sh num terminal (com eco do terminal, como uma pessoa usaria)
def interativo(binario_rel):
    import pty, select, signal, time
    pid, fd = pty.fork()
    if pid == 0:
        os.chdir(B)
        os.execve('/bin/bash', ['bash', 'shrdlu.sh'], dict(os.environ, MACLISP=binario_rel))
    buf = ''
    def ate(texto, inicio, prazo):
        nonlocal buf
        t0 = time.monotonic()
        while time.monotonic() - t0 < prazo:
            r, _, _ = select.select([fd], [], [], .2)
            if r:
                try:
                    d = os.read(fd, 16384)
                except OSError:
                    break
                if not d:
                    break
                buf += d.decode('latin-1')
            if texto in buf[inicio:]:
                return True
        return False
    ok = ate('READY/', 0, 60)
    i = len(buf)
    os.write(fd, b'PICK UP A BIG RED BLOCK.\r')
    ok = ate('READY/', i, 60) and ok
    resposta = ' '.join(buf[i:].replace('\r', ' ').split())
    os.kill(pid, signal.SIGKILL); os.waitpid(pid, 0)
    return ok and 'OK.' in resposta, resposta


if 'gcc' in resumo:
    print('\n== modo interativo (shrdlu.sh)', flush=True)
    ok, resp = interativo('build/maclisp-gcc')
    confere(ok, f'shrdlu.sh abre o SHRDLU e responde a uma frase digitada: {resp[:70]}')

# 5. compiladores entre si
if len(resumo) == 2:
    a, b = resumo['gcc']['dialogo_46']['sha256_dialogo'], resumo['clang_ubsan']['dialogo_46']['sha256_dialogo']
    confere(a == b, 'GCC e Clang/UBSan produzem o mesmo dialogo, byte a byte')

(RES / 'RESULTADOS.json').write_text(json.dumps({'resultado': 'OK' if not falhas else 'FALHOU',
                                                 'falhas': falhas, 'compiladores': resumo},
                                                ensure_ascii=False, indent=2) + '\n')
print('\nRESULTADO:', 'tudo conferido' if not falhas else f'{len(falhas)} falha(s)')
sys.exit(1 if falhas else 0)
