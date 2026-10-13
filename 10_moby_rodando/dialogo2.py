# Alimenta o SHRDLU original (moby dump) com o dialogo da tese, frase a frase, pelo console do ITS (tmux).
# Regras: depois da resposta o programa espera a proxima tecla; quando fica parado sem responder, um espaco o destrava.
import subprocess, time, os, sys, re
LOG = '/home/claude/work/its_tape/console.log'
OUT = sys.argv[2] if len(sys.argv) > 2 else '/home/claude/work/its_tape/dialogo_moby_v3.txt'
def key(*k): subprocess.run(['tmux', 'send-keys', '-t', 'its'] + list(k))
def send(s):
    for c in s:
        if c == ' ': key('Space')
        elif c == ';': key('-l', '--', '\;')
        else: key('-l', '--', c)
        time.sleep(0.05)
def size(): return os.path.getsize(LOG)
def since(m):
    with open(LOG, 'rb') as f: f.seek(m); return f.read().decode('latin-1').replace('\r', '')
lines = [l.strip() for l in open('/home/claude/work/kit/bateria/dialogo_1971_entrada.txt') if l.strip()]
lines = [l for l in lines if not l.startswith('@')]
start = int(sys.argv[1]) if len(sys.argv) > 1 else 1
out = open(OUT, 'a')
for i, l in enumerate(lines, 1):
    if i < start or str(i) in os.environ.get('PULAR', '').split(','): continue
    if i > int(os.environ.get('ATE', '999')): break
    text = l.upper()
    if text == 'DOES A STEEPLE': text = 'DOES A STEEPLE '
    m = size(); send(text)
    t0 = time.time(); last = time.time(); prev = ''; chose = spaced = False; brk = 0; nudges = 0; notes = []
    while time.time() - t0 < 600:
        time.sleep(1.5)
        new = since(m)
        if new != prev: last = time.time(); prev = new
        k = new.find('\n*'); body = new[k + 2:] if k >= 0 else ''
        quiet = time.time() - last
        if 'DO YOU MEAN' in body and not chose and quiet > 2:
            send('2'); chose = True; notes.append('respondeu 2'); continue
        if 'TWO SPACES' in body and not spaced and quiet > 2:
            send('  '); spaced = True; notes.append('digitou dois espacos'); continue
        if body.count('LISTENING') > brk and quiet > 2:
            brk = body.count('LISTENING'); send('NIL '); notes.append('parada do planner: NIL'); continue
        if 'READY' in body: break
        if re.search(r'\n\*[^\n]*[.?!]\s*$', body) and quiet > 5: break
        if quiet > 15 and nudges < 4 and k >= 0:
            send(' '); nudges += 1; last = time.time(); notes.append('espaco'); continue
        if quiet > 120: notes.append('PARADO'); break
    new = since(m)
    out.write('=== %d %s\n[%s]\n%s\n' % (i, l, ', '.join(notes), new)); out.flush()
    print(i, l, '->', ' | '.join(x.strip() for x in new.split('\n') if x.strip())[-230:], '[' + ', '.join(notes) + ']', flush=True)
    if 'PARADO' in notes: break
