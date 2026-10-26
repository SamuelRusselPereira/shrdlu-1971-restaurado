# 11 — Interpretador em C: o SHRDLU de 1971 sem emulador

*English version below.*

Um interpretador de MacLisp escrito em C99, num arquivo só (`maclisp.c`). Ele carrega os arquivos originais do SHRDLU da época da tese (os arquivos Z, "circa January 1971"; 7 dos 8, porque o oitavo é código de máquina da tela) e o Micro-Planner `plnr.176`, sem mudar nenhum byte deles. Com isso, o diálogo da tese roda num computador comum, sem o emulador do PDP-10.

Por cima dos originais vai uma camada de ajustes, cada um num arquivo separado e com o motivo no cabeçalho. Ela tem os ajustes publicados da rodada 16 (pasta `10_moby_rodando/`) e as funções que a imagem MOBY tinha compiladas de um jeito diferente do texto dos arquivos.

## O resultado

| Referência | Resultado |
|---|---|
| Rodada 16: o SHRDLU original (imagem MOBY) no PDP-10 emulado, pasta `10_moby_rodando/` | **46 de 46 respostas iguais** |
| Diálogo impresso na tese (AI-TR-235, seção 1.3, pp. 35–60) | **36 iguais** palavra por palavra, **5 com uma frase a mais** (3, 7, 8, 14, 26) e **5 diferentes** (2, 19, 24, 37, 46). É o mesmo placar do programa original no emulador. |
| 10 frases que não estão no diálogo da tese, digitadas no ITS e aqui | **10 de 10 respostas iguais**, inclusive as erradas ("I DON'T UNDERSTAND", "NOTHING THINGS.") e uma mensagem interna do programa |

- Regra de comparação: só letras e números, em maiúsculas. É a mesma regra das rodadas no emulador. Não é igualdade de bytes com o terminal do ITS.
- A numeração é a da tese, que pula o número 38. Da frase 39 em diante, a posição digitada é um número menor.
- Compilado com GCC 13 em C99 estrito, sem nenhum aviso, e com Clang 18 e o UndefinedBehaviorSanitizer. Os dois produzem o mesmo diálogo, byte a byte. Testado no Linux.
- As 10 diferenças em relação à tese são as mesmas do programa original no emulador. Estão explicadas uma a uma no README principal (seção "O SHRDLU da época da tese voltou a rodar") e em `../10_moby_rodando/RESULTADOS.md` (em inglês: `RESULTS_EN.md`).

## Como usar

1. **Testar tudo:** `python3 testar.py`. Leva uns 10 segundos. No fim deve aparecer `RESULTADO: tudo conferido`.
2. **Conversar com o SHRDLU:** `./shrdlu.sh` (ou `bash shrdlu.sh`). Na primeira vez ele compila o interpretador.
3. Espere `READY/` e digite a frase **em maiúsculas**, terminando com ponto ou interrogação. Por exemplo: `PICK UP A BIG RED BLOCK.`
4. Para sair, aperte Ctrl-C.

Para só compilar (o `shrdlu.sh` faz isso sozinho): `mkdir -p build && gcc -std=c99 -O2 -o build/maclisp maclisp.c -lm`.

Precisa de um compilador C (gcc ou clang) e, para os testes, de Python 3. A primeira linha da carga, `;PURE UNBOUND VARIABLE`, é esperada: vem do próprio `plnr.176` (linha 60), que testa a variável PURE dentro de um ERRSET.

## O que é carregado

A ordem está em `carga.txt`. São quatro partes.

1. **Históricos, intactos** (`historicos/`): o `plnr.176` e 7 dos 8 arquivos Z, mais o arquivo de dados `data.5`. O oitavo, `z.mover`, é código de máquina do PDP-10 para a tela 340 e o braço. Aqui não há tela: o `maclisp.c` define essas funções como vazias (a lista está no fim do `maclisp.c`).
2. **Apoio de 2026** (`apoio/`, 7 arquivos):
   - `boot1.lisp` e `macr_z.lisp`: preparam o Micro-Planner e os macros da gramática.
   - `reconstruidas.lisp`: funções que só existem compiladas na imagem, sem fonte em lugar nenhum.
   - `compiladas.lisp`: deixa uma cópia SUBR das funções que a imagem tinha compiladas, como na imagem.
   - `ambiente.lisp`: duas primitivas de terminal (LISTEN, SLEEP).
   - `variaveis.lisp`: as 4 variáveis da rodada 16.
   - `lote.lisp`: nas paradas do Micro-Planner, a resposta NIL que o operador digitava já vem embutida.
3. **Ajustes publicados da rodada 16** (`ajustes/rodada16/`): os mesmos 13 arquivos da pasta 10, byte a byte. Mais o remendo da gramática (`remendo_ng_no_texto.lisp`): no emulador ele muda duas palavras do código de máquina; aqui muda as mesmas duas expressões no texto da gramática.
4. **Como na imagem** (`ajustes/imagem/`, 6 arquivos): onde o código compilado da imagem MOBY é diferente do texto dos arquivos, o arquivo carrega a versão da imagem, lida no código de máquina. O endereço octal está no cabeçalho de cada um:
   - a função SHRDLU, a CONJ e o programa NG da gramática;
   - THRETURN (na imagem, o argumento fica na pilha);
   - o leitor de frases (ETAOIN e suas auxiliares);
   - três funções do Micro-Planner: THVAL, THFAIL e THGOALF.

## A frase 16

Na frase 16 ("DO I LIKE ANYTHING IN THE BOX?"), a tese responde "YES, TWO THINGS: THE BLUE BLOCK AND THE BLUE PYRAMID" (p. 42). Com o `THFAIL` do `plnr.176`, o C só achava a pirâmide.

- **Causa:** a imagem MOBY foi montada com um Micro-Planner mais antigo que o `plnr.176`. Dentro de um teorema, o `(THFAIL THGOAL)` da imagem descarta só o último subobjetivo. O do `plnr.176` derruba o objetivo que chamou o teorema, e então o teorema TH1 ("blocos que não são vermelhos") nunca é tentado.
- **A tese confirma o TH1:** na nota da p. 42, Winograd escreve "Presumably we like anything which is nice, and the blue block is certainly not red".
- **Os manuais mostram a ordem das versões.** Na descrição do THFAIL do manual do Micro-Planner de julho de 1970 (AIM-203, p. 15) não há "falha composta", e `(THFAIL THINF)` leva a falha até o topo do THVAL. Na do manual de dezembro de 1971 (AIM-203a, p. 14) aparecem a falha composta e o `THFAIL?`, que o `plnr.171` (fita de 5/10/1971) e o `plnr.176` (fita de 28/12/1971) implementam. A imagem não tem o símbolo `THFAIL?`, e o THVAL dela leva a falha do `(THFAIL THINF)` até o topo, como no manual de 1970.
- **Prova no ITS:**
  - Teste mínimo do THFAIL, numa sessão nova do MOBY: o ITS dá `(#FFF 7)`, e o C dá o mesmo.
  - Na frase 16, a ordem dos teoremas é `(TH2 TH1)` no ITS e no C. O rastro de 20 eventos do ITS é igual ao do C: 32 linhas idênticas, depois de juntar as linhas que o ITS quebra na coluna 72.
- **Controle:** sem `planner_como_na_imagem.lisp`, a frase 16 volta a divergir. O `testar.py` confere isso também.

Uma segunda sessão do ITS, feita de forma independente pelo ChatGPT, mostra a mesma ordem `(TH2 TH1)`, os mesmos teoremas e a mesma resposta (`evidencias/segunda_sessao_ITS_chatgpt/`).

## O que isto não é

- Não é a tese palavra por palavra: são 36 de 46, como o programa original no emulador.
- Não é o MacLisp completo de 1971: é o suficiente para rodar o SHRDLU, conferido pelo diálogo, por 10 frases a mais e pela frase 16 passo a passo.
- Não é "o código original sem nenhuma intervenção": os arquivos históricos ficam intactos no disco, mas a camada de ajustes troca funções na memória, depois da carga.
- Não é o SHRDLU reescrito em C: os arquivos Z continuam sendo interpretados. A tradução função por função para C é um trabalho separado, ainda por fazer.
- Não foi testado fora do Linux.

## Evidências

- `evidencias/its/`: os logs brutos do ITS (teste mínimo, frases 1 a 16 com a sonda, 10 frases fora do diálogo), os roteiros que os produziram e o passo a passo para repetir (`COMO_REPETIR_NO_ITS.md`).
- `evidencias/segunda_sessao_ITS_chatgpt/`: a segunda sessão do ITS.
- `evidencias/moby/`: as listagens do código de máquina da imagem (THVAL, THFAIL, THGOALF e as funções que elas chamam) e o script que as refaz a partir do `moby.dump`.
- `resultados/`: as saídas do `testar.py` nesta máquina.

A origem e o SHA-256 de cada arquivo estão em `PROVENIENCIA.md` e `SHA256SUMS`. As licenças estão em `NOTICE.md`.

Feito por Samuel Russel Pereira com o Claude (Anthropic) e o ChatGPT (OpenAI), em outubro de 2026.

---

# 11 — C interpreter: the 1971 SHRDLU without an emulator

A MacLisp interpreter written in C99, in a single file (`maclisp.c`). It loads the original thesis-era SHRDLU files (the Z files, "circa January 1971"; 7 of the 8, since the eighth is display machine code) and the Micro-Planner source `plnr.176` without changing a single byte, so the thesis dialogue runs on an ordinary computer, with no PDP-10 emulator.

On top of the originals there is a layer of adjustments, one file each, with the reason in its header. It holds the published round-16 adjustments (folder `10_moby_rodando/`) and the functions that the MOBY image had compiled differently from the text of the files.

## Result

| Reference | Result |
|---|---|
| Round 16: the original SHRDLU (MOBY image) on the emulated PDP-10, folder `10_moby_rodando/` | **46 of 46 answers identical** |
| Dialogue printed in the thesis (AI-TR-235, section 1.3, pp. 35–60) | **36 identical** word for word, **5 with an extra sentence** (3, 7, 8, 14, 26), **5 different** (2, 19, 24, 37, 46): the same score as the original program on the emulator |
| 10 sentences that are not in the thesis dialogue, typed on ITS and here | **10 of 10 answers identical**, including the wrong ones and an internal message of the program |

Comparison rule: letters and digits only, upper case (the same rule as the emulator rounds); it is not byte equality with the ITS terminal. Numbering is the thesis's own (it has no item 38). The 10 differences from the thesis are those of the original program on the emulator, explained one by one in `../10_moby_rodando/RESULTS_EN.md`. Compiled with GCC 13 in strict C99 with no warnings, and with Clang 18 and the UndefinedBehaviorSanitizer; both produce the same dialogue byte for byte. Tested on Linux.

## How to use

1. **Run all tests:** `python3 testar.py` (about 10 seconds). It must end with `RESULTADO: tudo conferido`.
2. **Talk to SHRDLU:** `./shrdlu.sh` or `bash shrdlu.sh` (it compiles the interpreter the first time). Wait for `READY/` and type the sentence **in upper case**, ending with a period or question mark, e.g. `PICK UP A BIG RED BLOCK.` Quit with Ctrl-C.

To compile only (`shrdlu.sh` does it by itself): `mkdir -p build && gcc -std=c99 -O2 -o build/maclisp maclisp.c -lm`. Needs a C compiler (gcc or clang) and, for the tests, Python 3. The first load message, `;PURE UNBOUND VARIABLE`, is expected: it comes from `plnr.176` itself (line 60), which tests PURE inside an ERRSET.

## What is loaded (`carga.txt`)

1. **Historical files, untouched** (`historicos/`): `plnr.176`, 7 of the 8 Z files and the data file `data.5`. The eighth, `z.mover`, is PDP-10 machine code for the 340 display and the arm; there is no display here, and `maclisp.c` defines those functions as no-ops.
2. **2026 support** (`apoio/`, 7 files): start-up and grammar macros, functions that exist only compiled in the image (no source anywhere), SUBR copies of the functions the image had compiled, two terminal primitives, the 4 round-16 variables, and batch handling of Micro-Planner breaks (the operator's NIL answer built in).
3. **Published round-16 adjustments** (`ajustes/rodada16/`): the same 13 files as folder 10, byte for byte, plus the grammar patch applied to the grammar text instead of the machine code.
4. **As in the image** (`ajustes/imagem/`, 6 files): where the compiled code of the MOBY image differs from the text of the files, the version read from the image's machine code (octal addresses in each header): SHRDLU, CONJ, the NG grammar program, THRETURN, the sentence reader (ETAOIN) and three Micro-Planner functions, THVAL, THFAIL and THGOALF.

## Sentence 16

The thesis answers "YES, TWO THINGS: THE BLUE BLOCK AND THE BLUE PYRAMID" (p. 42); with the `THFAIL` of `plnr.176`, the C run found only the pyramid.

- The MOBY image was built with an older Micro-Planner than `plnr.176`. Inside a theorem, the image's `(THFAIL THGOAL)` discards only the last subgoal; the one in `plnr.176` kills the goal that called the theorem, so theorem TH1 ("blocks which are not red") is never tried.
- The thesis note on p. 42 relies on TH1: "Presumably we like anything which is nice, and the blue block is certainly not red."
- The manuals show the order of the versions: the THFAIL entry of the July 1970 Micro-Planner manual (AIM-203, p. 15) has no "compound failure", and `(THFAIL THINF)` takes the failure to the top level of THVAL; the entry of the December 1971 manual (AIM-203a, p. 14) has compound failures and `THFAIL?`, which `plnr.171` (tape date 1971-10-05) and `plnr.176` (tape date 1971-12-28) implement. The image has no `THFAIL?` symbol, and its THVAL takes a `(THFAIL THINF)` failure to the top level, as in the 1970 manual.
- On ITS: a minimal THFAIL test in a fresh MOBY session gives `(#FFF 7)`, as the C run does. In sentence 16 the theorem order is `(TH2 TH1)` on ITS and in C, and the 20-event trace is identical (32 lines, after rejoining the lines ITS wraps at column 72).
- Control: without `planner_como_na_imagem.lisp`, sentence 16 diverges again; `testar.py` checks this too.

A second ITS session, made independently by ChatGPT, shows the same order, the same theorems and the same answer (`evidencias/segunda_sessao_ITS_chatgpt/`).

## What this is not

Not the thesis word for word (36 of 46, like the original program on the emulator). Not the complete 1971 MacLisp (enough to run SHRDLU, checked by the dialogue, 10 extra sentences and sentence 16 step by step). Not "the original code with no intervention": the historical files stay untouched on disk, but the adjustment layer replaces functions in memory after loading. Not SHRDLU rewritten in C: the Z files are still interpreted; a function-by-function C translation is separate, future work. Not tested outside Linux.

## Evidence, provenance, licenses

`evidencias/its/` (raw ITS logs, the scripts that produced them, and how to repeat them), `evidencias/segunda_sessao_ITS_chatgpt/`, `evidencias/moby/` (machine-code listings of the image and the script that regenerates them from `moby.dump`), `resultados/` (outputs of `testar.py`). Origin and SHA-256 of every file: `PROVENIENCIA.md` and `SHA256SUMS`. Licenses: `NOTICE.md`.

Made by Samuel Russel Pereira with Claude (Anthropic) and ChatGPT (OpenAI), October 2026.
