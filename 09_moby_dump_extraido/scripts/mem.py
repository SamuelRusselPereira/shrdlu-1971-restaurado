# Le a saida do dis10 (-Wits) e monta o mapa de memoria: endereco -> palavra de 36 bits
import os
import re, sys, pickle
def load(path):
    mem = {}
    pat = re.compile(r'^([0-7]{6}):\s+([0-7]{12})')
    for line in open(path, encoding='latin-1'):
        m = pat.match(line)
        if m: mem[int(m.group(1), 8)] = int(m.group(2), 8)
    return mem
def a5(s):  # 5 caracteres ASCII de 7 bits, alinhados a esquerda (formato de pname)
    s = (s + '\0'*5)[:5]; w = 0
    for c in s: w = (w << 7) | ord(c)
    return w << 1
def chars(w):
    return ''.join(chr((w >> (29 - 7*i)) & 0o177) for i in range(5))
LH = lambda w: w >> 18
RH = lambda w: w & 0o777777
if __name__ == '__main__':
    mem = load(sys.argv[1]); pickle.dump(mem, open(sys.argv[2], 'wb'))
    print(len(mem), 'palavras; faixa', oct(min(mem)), '-', oct(max(mem)))
