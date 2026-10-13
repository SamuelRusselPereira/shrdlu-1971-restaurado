# Como rodar o moby dump (SHRDLU da época da tese, 1970–71) no emulador do PDP-10

Testado em 10–13/10/2026 num Linux (Ubuntu), com o emulador KA10 do projeto PDP-10/its.

## 1. Ferramentas
- Pacotes: git, gcc, make, expect, tcl, libsdl2-dev, libsdl2-image-dev, libsdl2-net-dev, tmux.

## 2. Emulador e imagem pronta do ITS
- `git clone --depth 1 https://github.com/PDP-10/its.git && cd its`
- `git submodule update --init --depth 1 tools/sims tools/itstar`
- `make -C tools/sims pdp10-ka` (o emulador KA10)
- `curl -O http://hactrn.kostersitz.com/images/pdp10-ka.tgz && tar xzf pdp10-ka.tgz`
- Guarde o `pdp10-ka.tgz`. Se o disco do ITS estragar, extraia de novo só os arquivos `rp03.*`.
- `mkdir -p out/pdp10-ka` e crie links em `out/pdp10-ka/` para `rp03.0`–`rp03.3` e `dskdmp.rim` de `pdp10-ka/`.

## 3. Fita com o diretório SHRDLU antigo
- `make -C tools/itstar`
- Copie os arquivos de `7_its_vault_SHRDLU_original/` para uma pasta `oshrdl/`.
- `tools/itstar/itstar -cf oshrdl.tape -C . oshrdl`
- Ajuste o caminho da fita no `run.ini` (linha `at mta0 ...`).

## 4. Ligar o ITS
- `tools/sims/BIN/pdp10-ka run.ini`
- Apareceu `DSKDMP`? Espere uns 5 segundos antes de digitar. Letras digitadas cedo demais se perdem, e o DSKDMP responde `FNF`.
- Digite `its` e Enter; depois `styo+2/popj p,` e Enter; depois ESC e `g`.
- Espere `IN OPERATION` e aperte Ctrl-Z.
- `:login seunome`

## 5. Copiar a fita para o disco do ITS (só na primeira vez)
- `:dump` → `load links crdir sorry` → no `FILE=` digite `*;* *` → espere `E-O-T` → `quit`
- Confira com `:listf oshrdl;` (deve aparecer `MOBY DUMP`).

## 6. Carregar e iniciar o moby dump
- `:job moby` → `:load oshrdl;moby dump` → ESC e `g`
- Digite, uma por vez: `(SETQ NOSTOP T)` `(SETQ NOMEM NIL)` `(SETQ PLANNERSEE NIL)` `(SETQ VALUE NIL)`
- Digite a definição de `SMNGOF` que está em `smngof_z.lisp`. É a versão do próprio Winograd, dos arquivos Z. A da imagem trava o Micro-Planner na frase 7.
- Digite as duas linhas de `fixes_ambiente.lisp`. Elas fazem READCH devolver dígitos como números, como o código espera.
- Digite a linha de `findq_reconstruida.lisp`. **Esta é uma reconstrução minha**, não código de 1971 (ver `RESULTADOS.md`). Sem ela a frase 5 falha.
- Digite as linhas de `smone_corrigida.lisp`, `smprop_corrigida.lisp`, `smadj_corrigida.lisp`, `modal_zerado.lisp` e `goalize2_corrigida.lisp`. **São correções minhas** (ver `RESULTADOS.md`). Sem elas, as frases 13 a 16 e 42 a 45 falham.
- Digite as duas linhas de `remendo_ng.lisp`. Elas mudam duas instruções da gramática compilada, com `DEPOSIT` (ver `RESULTADOS.md`). Sem elas, as frases 28 a 33 falham.
- Digite as linhas de `smit3_corrigida.lisp` e `who_corrigida.lisp`. **São mudanças minhas** (ver `RESULTADOS.md`). Sem elas, a frase 23 pergunta qual "it" em vez de responder.
- Digite a linha de `loc_thmlist_corrigida.lisp`. Ela devolve à relação #LOC a lista de teoremas da linha 892 de `z.dictio` (ver `RESULTADOS.md`). Sem ela, a frase 35 acha zero blocos.
- Digite as linhas de `smrsq_corrigida.lisp`, `smng1_corrigida.lisp` e `size_corrigida.lisp`. **São correções minhas**, feitas como nas versões seguintes do programa (ver `RESULTADOS.md`). Sem elas, a frase 37 dá erro.
- `(SHRDLU)`
- Aparece `READY`. Digite as frases em MAIÚSCULAS, terminando com `.` ou `?`.

## Cuidados
- **Nunca rode dois emuladores ao mesmo tempo com o mesmo disco.** Os dois escrevem nos arquivos `rp03.*` e o disco do ITS estraga. Aconteceu aqui uma vez; o disco foi refeito a partir do `pdp10-ka.tgz`.
- Fechar a janela do tmux não fecha o emulador. Ele continua rodando escondido. Antes de ligar outro, feche o processo: `pkill -x pdp10-ka`.

## Observações
- `NOMEM NIL` liga a memória de eventos (vem desligada na imagem).
- `VALUE NIL` contorna um detalhe do MacLisp de 1971: o segundo elemento de uma lista vazia não é vazio, é o símbolo VALUE. Sem isso a frase 3 dá erro.
- Às vezes o programa para depois do `*` sem responder (parece esperar a tela 340, que não temos). Um espaço destrava.
- Em `>>> ... LISTENING` (parada do Micro-Planner), digite `NIL` e espaço: o objetivo falha e o programa segue.
- Ctrl-G volta ao nível de cima do Lisp. Dali dá para redefinir uma função e voltar com `(SHRDLU)`. No teste feito aqui, o estado do mundo e do diálogo continuou.

## Scripts
- `its.sh`: fala com o console do ITS dentro do tmux (`start`, `send`, `cr`, `esc`, `ctrl`, `cap`, `stop`). O `stop` e o `start` matam o emulador antigo.
- `restaura_disco.sh`: liga o ITS com o disco limpo e copia a fita para o disco (passo 5).
- `boot_moby.sh`: liga o ITS, entra e carrega a imagem com as chaves do diálogo.
- `dialogo2.py`: digita o diálogo da tese frase a frase e grava a transcrição. Responde "2" à pergunta de esclarecimento, dá os dois espaços pedidos e manda um espaço quando o programa fica parado.
- `compara.py`: compara uma transcrição com a tese, frase a frase, pela posição.
- `rodada16_completa.sh`: a rodada 16 inteira, a definitiva (liga, carrega todos os ajustes, digita as 46 frases). `rodada10_completa.sh` a `rodada15_completa.sh` são as anteriores.
- `ponto12.sh`, `ponto22.sh`, `ponto27.sh`, `ponto34.sh`, `ponto36.sh` e `ponto36b.sh`: rodam até a frase 12, 22, 27, 34 ou 36 e salvam o job como ponto de controle (`:pdump oshrdl;s12 dump`). `carrega_ponto.sh s12` recarrega. Cada um carrega os ajustes que existiam quando foi feito: o `ponto36b.sh` tem todos os da rodada 14, e o `ponto34.sh`, os da rodada 13; os mais antigos não têm os que vieram depois.
- `trace_p1.lisp`, `trace_ans1.lisp`, `trace_ansthm.lisp`: rastreios que imprimem os argumentos de P1, ANS1 e ANSTHM (carregue depois de ligar o rastreio).
- `shrdlu2.lisp`: volta ao diálogo sem apagar a conversa (a função `(SHRDLU)` apaga).
- `trace_defs.lisp` e `trace_lista.lisp`: o rastreio. Depois de carregá-los, `(MAPCAR (FUNCTION TR) TRLIST)` liga e `(MAPCAR (FUNCTION UNTR) TRLIST)` desliga.
- `trace_valores.lisp`: rastreio que imprime também os argumentos e o valor de cada chamada (`TRV`). Depois de carregá-lo, ligue nas funções que interessam, por exemplo `(MAPCAR (FUNCTION TRV) (QUOTE (ANS1 ANSX THVAL2)))`.
- `asmuch_teste_37.lisp`: teste do teorema de "as wide as" (TC-ASMUCH). **Não use na rodada normal**: não muda nenhuma resposta, e o erro que ele corrige existe também na versão reescrita.
- `dialogo_1971_entrada.txt` (as frases que o roteiro digita) e `dialogo_1971.tsv` (as respostas da tese, com página).
- Os scripts usam os caminhos desta máquina (`/home/claude/...`). Ajuste a linha `T=` de cada um e os caminhos no começo de `dialogo2.py` e `compara.py`.
