#!/usr/bin/env python3
"""Digita frases no SHRDLU rodando no interpretador em C, uma de cada vez, como o dialogo2.py fazia no emulador
(../10_moby_rodando/dialogo2.py): responde 2 quando o programa pergunta "DO YOU MEAN" e digita dois espacos
quando ele pede "TWO SPACES".

Types sentences into SHRDLU running on the C interpreter, one at a time, like dialogo2.py did on the emulator.

Uso / usage:
  python3 testes/dialogo.py --maclisp build/maclisp-gcc --saida resultados/dialogo.txt
  opcoes: --frases ARQ (padrao: testes/dialogo_1971_entrada.txt)  --max N
          --sem ARQ (tira um arquivo da carga)  --extra ARQ (carrega mais um arquivo no fim)
Grava a transcricao em --saida e o estado de cada frase em --saida + '.status'.
"""
import argparse, os, pty, select, signal, sys, termios, time
from pathlib import Path

B = Path(__file__).resolve().parents[1]


def carga(sem=(), extra=()):
    """Arquivos de carga.txt, na ordem, com caminhos relativos a esta pasta (o interpretador roda nela)."""
    arqs = []
    for linha in (B / 'carga.txt').read_text().splitlines():
        linha = linha.strip()
        if not linha or linha.startswith('#') or linha in sem:
            continue
        arqs.append(linha)
    return arqs + [os.path.relpath(Path(x).resolve(), B) for x in extra]


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--maclisp', default=str(B / 'maclisp'))
    p.add_argument('--saida', required=True)
    p.add_argument('--frases', default=str(B / 'testes/dialogo_1971_entrada.txt'))
    p.add_argument('--max', type=int, default=999)
    p.add_argument('--timeout', type=float, default=30.0)
    p.add_argument('--sem', action='append', default=[])
    p.add_argument('--extra', action='append', default=[])
    a = p.parse_args()
    frases = [x.strip().upper() for x in Path(a.frases).read_text().splitlines()
              if x.strip() and not x.startswith('@')][:a.max]
    maclisp = str(Path(a.maclisp).resolve())
    args = [maclisp, *carga(a.sem, a.extra), '-e', '(SHRDLU)']
    pid, fd = pty.fork()
    if pid == 0:
        attr = termios.tcgetattr(0)
        attr[3] &= ~termios.ECHO            # o terminal nao ecoa; o interpretador ecoa o que le (ECHO=1)
        termios.tcsetattr(0, termios.TCSANOW, attr)
        os.chdir(B)
        os.execve(maclisp, args, dict(os.environ, ECHO='1'))
    buf = ''
    estados = []
    with open(a.saida, 'w', encoding='latin-1') as out:
        def ler(espera):
            nonlocal buf
            r, _, _ = select.select([fd], [], [], espera)
            if not r:
                return False
            try:
                s = os.read(fd, 16384)
            except OSError:
                return False
            if not s:
                return False
            t = s.decode('latin-1')
            buf += t
            out.write(t)
            out.flush()
            return True

        def enviar(linha):
            os.write(fd, (linha + '\r').encode('latin-1'))

        def esperar_ready(inicio):
            ultimo = time.monotonic()
            prazo = ultimo + a.timeout
            pediu2 = espacos = False
            while time.monotonic() < prazo:
                if ler(.15):
                    ultimo = time.monotonic()
                corpo = ' '.join(buf[inicio:].upper().split())   # a mensagem pode quebrar de linha
                if 'READY/' in corpo:
                    return 'READY'
                quieto = time.monotonic() - ultimo
                if 'DO YOU MEAN' in corpo and not pediu2 and quieto > .3:
                    enviar('2'); pediu2 = True; prazo = max(prazo, time.monotonic() + a.timeout)
                if 'TWO SPACES' in corpo and not espacos and quieto > .3:
                    enviar('  '); espacos = True; prazo = max(prazo, time.monotonic() + a.timeout)
            return 'TIMEOUT'

        estado = esperar_ready(0)
        if estado == 'READY':
            for n, frase in enumerate(frases, 1):
                inicio = len(buf)
                enviar(frase + ' ' if frase == 'DOES A STEEPLE' else frase)   # como no dialogo2.py
                t0 = time.monotonic()
                estado = esperar_ready(inicio)
                estados.append((n, estado, round(time.monotonic() - t0, 2), frase))
                if estado != 'READY':
                    break
        else:
            print('o SHRDLU nao chegou ao primeiro READY/', file=sys.stderr)
        Path(a.saida + '.status').write_text(''.join('\t'.join(map(str, e)) + '\n' for e in estados))
        try:
            os.kill(pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        try:
            os.waitpid(pid, 0)
        except ChildProcessError:
            pass
    ok = sum(1 for e in estados if e[1] == 'READY')
    print(f'{ok} de {len(frases)} frases chegaram a READY/')
    sys.exit(0 if ok == len(frases) else 1)


if __name__ == '__main__':
    main()
