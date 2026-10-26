# Desmontador de PDP-10 para a memoria da imagem moby (moby.pkl, atoms.pkl). Uso: python3 desmonta.py ENDERECO_OCTAL N
# moby.pkl e atoms.pkl saem de 07_its_vault_SHRDLU_original/moby.dump com dis10 -Wits -Snone, mem.py e atoms.py
# (repositorio shrdlu-1971-restaurado, pasta 09_moby_dump_extraido/scripts). Caminhos: variaveis MOBY_PKL e ATOMS_PKL.
import os,pickle,sys
AQUI=os.path.dirname(os.path.abspath(__file__))
m=pickle.load(open(os.environ.get('MOBY_PKL',os.path.join(AQUI,'moby.pkl')),'rb'))
at,byname=pickle.load(open(os.environ.get('ATOMS_PKL',os.path.join(AQUI,'atoms.pkl')),'rb'))
vcell={}
for a,name in at.items():
    w=m.get(a,0); p=w&0o777777
    for i in range(14):
        if not p: break
        c=m.get(p,0); car=c>>18; c2=m.get(c&0o777777,0)
        if at.get(car)=='VALUE': vcell[c2>>18]=name
        p=c2&0o777777
ops={0o200:'MOVE',0o201:'MOVEI',0o202:'MOVEM',0o554:'HLRZ',0o550:'HRRZ',0o260:'PUSHJ',0o261:'PUSH',0o262:'POP',0o263:'POPJ',0o254:'JRST',0o322:'JUMPE',0o326:'JUMPN',0o14:'CALL',0o15:'JCALL',0o16:'CALLF',0o17:'JCALLF',0o332:'SKIPE',0o336:'SKIPN',0o312:'CAME',0o316:'CAMN',0o302:'CAIE',0o306:'CAIN',0o274:'SUB',0o271:'ADDI',0o270:'ADD',0o211:'MOVNI',0o250:'EXCH',0o213:'MOVNS',0o402:'SETZM',0o403:'SETZB',0o506:'HRLM',0o542:'HRRM',0o553:'HRRZS',0o634:'TDZA',0o265:'JSP',0o344:'AOJA',0o350:'AOS',0o370:'SOS',0o505:'HRLI',0o541:'HRRI',0o551:'HRRZI',0o515:'HRLZI',0o561:'HRROI'}
SYS={0o6214:'NCONS',0o6215:'XCONS',0o6216:'CONS?',0o6355:'GET',0o7231:'ADD1',0o3572:'ERRSET',0o6320:'ASSQ',0o6323:'ASSOC?',0o7430:'LSUBRCALL',0o200:'RET'}
def lst(p,d=0):
    if p==0: return 'NIL'
    if p in at: return at[p]
    if d>4: return '..'
    w=m.get(p,0); car=w>>18; cdr=w&0o777777
    items=[];n=0
    while True:
        items.append(lst(car,d+1)); n+=1
        if cdr==0: return '('+' '.join(items)+')'
        if cdr in at: return '('+' '.join(items)+' . '+at[cdr]+')'
        if n>12: return '('+' '.join(items)+' ...)'
        w=m.get(cdr,0); car=w>>18; cdr=w&0o777777
def dis(a,n):
    for k in range(a,a+n):
        w=m.get(k,0); y=w&0o777777; op=w>>27; ac=(w>>23)&15; ind='@' if (w>>22)&1 else ''; ix=(w>>18)&15
        name=ops.get(op,oct(op))
        if ix==12: arg='%d(P)'%(y-0o1000000 if y>0o400000 else y)
        else:
            arg=oct(y)
            if y in at: arg="'"+at[y]
            elif y in vcell: arg='['+vcell[y]+']'
            elif op in (0o260,) and y in SYS: arg=SYS[y]
            elif op==0o201 and y>0o100000 and y not in at: arg=oct(y)+'='+lst(y)
        print(oct(k)[2:], f"{w:012o}", name, ac, ind+arg)
def props(nome):   # lista de propriedades do atomo: indicador e valor (SUBR/FSUBR/LSUBR = endereco do codigo)
    for a in byname.get(nome, []):
        l = m[a] & 0o777777; out = [nome, oct(a)[2:]]; n = 0
        while l and n < 40:
            ind = at.get(m.get(l, 0) >> 18, '?'); l2 = m.get(l, 0) & 0o777777; val = m.get(l2, 0) >> 18
            if ind != 'PNAME':
                if ind in ('SUBR', 'FSUBR', 'LSUBR'): out += [ind, oct(val)[2:]]
                elif ind in ('EXPR', 'FEXPR', 'MACRO'): out += [ind, '<lista>']
                else: out += [ind, ("'" + at[val]) if val in at else oct(val)[2:]]
            l = m.get(l2, 0) & 0o777777; n += 1
        print(' '.join(out))
if sys.argv[1] == '--props':   # python3 desmonta.py --props NOME1 NOME2 ... : enderecos de entrada das funcoes
    for nome in sys.argv[2:]: props(nome)
elif sys.argv[1] == '--lista':   # python3 desmonta.py --lista END1 END2 ... : imprime a lista Lisp guardada em cada endereco
    for e in sys.argv[2:]: print(e, lst(int(e, 8)))
else:
    dis(int(sys.argv[1],8),int(sys.argv[2]))
