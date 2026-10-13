# O SHRDLU de 1971 (moby dump) diante do diálogo da tese

**Rodada 16 (13/10/2026): ver a seção no fim. Rodada 15 (12/10/2026).** Sessão nova, imagem recém-carregada, as 46 frases do diálogo na ordem da tese, digitadas por `dialogo2.py`. Transcrição: `transcricoes/rodada15_definitiva.txt`. A tese não tem o número 38; a numeração abaixo é a dela.

## Placar

| rodada | o que mudou | iguais | quase | erradas |
|---|---|---|---|---|
| 4 | só `VALUE NIL` (22 frases tentadas) | 8 | 2 | 12 |
| 8 | + SMNGOF dos arquivos Z + ajuste do READCH | 24 | 4 | 18 |
| 9 | + FINDQ reconstruída | 25 | 4 | 17 |
| 10 | + SMONE e SMPROP corrigidas | 29 | 4 | 13 |
| 11 | + SMADJ corrigida, MODAL zerado, negados por último | 32 | 5 | 9 |
| 12 | + dois remendos na gramática compilada | 38 | 5 | 3 |
| 13 | + SMIT3 e WHO corrigidas | 39 | 5 | 2 |
| 14 | + lista de teoremas de #LOC | 40 | 5 | 1 |
| 15 | + SMRSQ, SMNG1 e SIZE corrigidas | **40** | 6 | **0** |

- "Iguais" inclui 5 respostas com um texto a mais além da resposta certa (frases 3, 7, 8, 14 e 26).
- Cada rodada repetiu a anterior em todas as frases, menos as que dependiam das correções novas (rodada 10: 42 a 46; rodada 11: 13 a 16; rodada 12: 28 a 33; rodada 13: só a 23; rodada 14: só a 35; rodada 15: só a 37). Nenhuma frase piorou.

## Ajustes usados

| ajuste | por quê | de onde vem |
|---|---|---|
| `(SETQ VALUE NIL)` | `THMLIST` pede `(CADR NIL)`. Neste MacLisp isso devolve o símbolo VALUE, e o programa tenta avaliá-lo. | contorno de ambiente |
| `SMNGOF` (`smngof_z.lisp`) | A SMNGOF guardada na imagem é mais antiga: entrega a variável ao Micro-Planner sem o marcador `THV`. Resultado: `THUNBOUND THGAL` na frase 7. | código do próprio Winograd (`z.semant`) |
| `READCH` (`fixes_ambiente.lisp`) | Neste MacLisp, READCH devolve o símbolo `2`. O código compara com o número 2, e a resposta "2" à pergunta de esclarecimento nunca casa. | contorno de ambiente |
| `FINDQ` (`findq_reconstruida.lisp`) | É chamada em `z.semant` (linha 984) e não está definida em nenhum arquivo nem na imagem. Sem ela a frase 5 falha. | **reconstrução minha** (abaixo) |
| `SMONE` (`smone_corrigida.lisp`) | Uma chamada tem um `LIST` a menos que a chamada vizinha. Sem a correção, "build one." (42) dá `CAR MADE ILGL MEM REF`. | **correção minha** de um erro no código de 1971 (abaixo) |
| `SMPROP` (`smprop_corrigida.lisp`) | Um `(OR (LIST …) …)` sempre escolhe o primeiro ramo e põe um NIL entre as interpretações de um nome próprio. Sem a correção, a frase 43 dá `CADAAR MADE ILGL MEM REF`. | **correção minha** de um erro no código de 1971 (abaixo) |
| `SMADJ` (`smadj_corrigida.lisp`) | Adjetivo depois de "is" ("the blue pyramid is nice"). Três defeitos no mesmo trecho (abaixo). Sem a correção, as frases 13 e 14 dão erro. | **correção minha** (abaixo) |
| MODAL zerado (`modal_zerado.lisp`) | A variável MODAL liga nas perguntas com "can" e nunca é desligada. Depois delas, um "não" vira "I DON'T KNOW" (frase 15). | **contorno meu** (abaixo) |
| `GOALIZE2` (`goalize2_corrigida.lisp`) | Um objetivo negado ("não é vermelho") recebe a mesma prioridade do positivo e é testado antes de existir o objeto (frase 16). | **correção minha** (abaixo) |
| gramática compilada (`remendo_ng.lisp`) | A gramática lê todo "that" como pronome interrogativo. Sem o remendo, as frases 28 a 33 falham. | **correção minha**, feita no código compilado (abaixo) |
| `SMIT3` (`smit3_corrigida.lisp`) | Procura o sintagma da frase anterior a que "it" pode se referir. Quando o sintagma fecha a frase, ela devolve outro. Na frase 23, entra "the tallest pyramid's support" no lugar de "anything green". | **correção minha** (abaixo) |
| `WHO` (`who_corrigida.lisp`) | O teste "ele já sabia a resposta?" recusa o fato "verde é uma cor", que nunca é mencionado. Em pergunta de cor, o teste nunca dá certo. | **mudança minha** (abaixo) |
| THMLIST de #LOC (`loc_thmlist_corrigida.lisp`) | A lista que diz qual teorema calcula "à esquerda de", "atrás de" etc. aparece duas vezes em `z.dictio`. A segunda apaga a primeira, e some a entrada com tempo. Sem ela, a frase 35 acha zero blocos. | **correção minha**, com o valor da própria linha 892 (abaixo) |
| `SMRSQ` (`smrsq_corrigida.lisp`) | Junta as orações relativas ligadas por "and". Com "but", dava `CAR MADE ILGL MEM REF` (frase 37). | **correção minha**, como na versão seguinte (abaixo) |
| `SMNG1` (`smng1_corrigida.lisp`) | "anything" não gerava candidatos. Na frase 37, a busca ficava sem objeto e o Micro-Planner parava. | **correção minha**, como nas versões seguintes (abaixo) |
| `SIZE` (`size_corrigida.lisp`) | Não conhecia a mão do robô. Com "anything" gerando candidatos, a mão entra na comparação de tamanho e dá erro. | **correção minha**, como na versão reescrita (abaixo) |

## A FINDQ reconstruída

- A chamada fica em SMCL1: `(SM (FINDQ (* H PV (QUEST) DLC)))`. Ela roda quando a pergunta termina em preposição ("what is the pyramid supported by?").
- Rastreei no emulador o que a FINDQ recebe nessa frase. O caminho `(* H PV (QUEST) DLC)` acha o sintagma "what" (`NG QUEST DET QDET NS DEF`) e desce um nível, até a palavra (`PRONREL NPL NS`). Uma palavra não tem semântica, e a frase falha. → `transcricoes/rastreio_FINDQ.txt`
- Pela gramática, quando a preposição vem na frente ("on which block…"), o mesmo caminho para no sintagma nominal certo.
- A reconstrução cobre os dois casos. Se recebeu um sintagma nominal, devolve-o. Se não, volta ao constituinte marcado com QUEST:
  `(DEFUN FINDQ (X) (COND ((ISX X (QUOTE NG)) X) ((* H PV (QUEST)))))`
- Com ela, a frase 5 responde "THE BOX.", igual à tese. Isso não prova que seja o código de Winograd. Mostra só que a peça que falta tem esse papel.

## A correção da SMONE ("one")

- O rastreio de "build one." mostra a falha dentro de NOUNCUT, chamada por SMONE2, chamada por SMONE. → `transcricoes/rastreio_BUILD_ONE.txt`
- A SMONE procura o substantivo a que "one" se refere. O último recurso é olhar a frase anterior: `(SMONE2 (LIST NIL (NB PT) (N PT)))`. Poucas linhas acima, a mesma chamada é `(SMONE2 (LIST (LIST NIL (NB PT) N)))`. A SMONE2 espera uma lista de trios; a última chamada passa um trio solto.
- Com o trio solto, a SMONE2 lê `(CADR NIL)`, que neste MacLisp é o símbolo VALUE, e NOUNCUT faz CAR no cabeçalho do átomo: o endereço 777777 não existe.
- As versões reescritas depois (`smspec.88` e `smspec.89` do its-vault, `smspec.97`) usam a mesma forma nas duas chamadas: `(SMONE2 (LIST (CAR PT)))`.
- Correção: `(SMONE2 (LIST (LIST NIL (NB PT) (N PT))))`, um `LIST` a mais. Com ela, "build one." responde "OK." e o robô constrói a torre.

## A correção da SMPROP (nomes próprios)

- O rastreio de 'call the biggest block "superblock".' mostra a falha dentro de CHECK, na segunda volta do laço de CMEANS sobre as interpretações de "superblock". → `transcricoes/rastreio_CALL_SUPERBLOCK.txt`
- A SMPROP monta essas interpretações: `(CONS NOME (OR (LIST (EVAL (GET palavra 'PROPN))) (MAPCAR …)))`. Um `(LIST …)` nunca é NIL, então o OR sempre fica com o primeiro ramo. Nenhuma palavra do dicionário tem a propriedade PROPN, e o resultado é `(NOME NIL)`: uma interpretação vazia. O segundo ramo, que procura o objeto que já recebeu esse nome, nunca roda.
- Correção: uma guarda, `(OR (AND (GET palavra 'PROPN) (LIST (EVAL …))) (MAPCAR …))`.
- Com ela, a 43 responde "OK.", e a 44 ("YES.") e a 45 também acertam: "superblock" passa a encontrar o bloco que recebeu o nome. A versão reescrita faz o mesmo de outro jeito: sempre duas interpretações, o nome e o objeto que tem esse nome.

## A correção da SMADJ (adjetivo depois de "is")

O rastreio da frase 13 mostrou três defeitos no mesmo trecho da SMADJ, o que monta a relação de "X is ADJ". → `transcricoes/rastreio_frases13a16.txt`

1. **Erro de digitação.** O MAPCAR que copia os elementos da relação devolve `X` (a estrutura inteira) onde devia devolver `Y` (o elemento). Em vez de `(#LIKE :FRIEND :B4)`, saía `(X X X)`.
2. **Símbolo errado.** O trecho usa `(CAR LIST1)` como se fosse o símbolo do sintagma, mas, duas linhas antes, LIST1 foi trocada pelas cópias feitas por REMPROVE. A SMRSQ precisa do símbolo (testa com EQ). Uso o último elemento da cópia, que é o símbolo original.
3. **Formato.** A SMADJ devolve o formato final, mas a etapa seguinte (SMCL2) converte de novo, e a relação sai um nível fora do lugar. Pelo código, o mesmo caminho recebe dos grupos preposicionais ("the block is in the box") o formato do BUILD. Passei a devolver esse formato.

Com isso, a 13 responde "I UNDERSTAND." e a 14 grava as duas preferências. A 14 imprime "I UNDERSTAND." duas vezes: a gramática responde a cada metade da frase separadamente.

## MODAL zerado depois de cada resposta

- A variável MODAL liga quando a pergunta tem "can". Ela faz um "não" virar "I DON'T KNOW", o que é certo para "can a pyramid support a pyramid?" (frase 11).
- No código de 1971, só a SMVG mexe em MODAL (`z.semant`, linhas 1677–1684), e só em frases com "can" ou "will". Nenhuma linha a desliga no fim da frase.
- Por isso, "do i like the box?" (15) respondia "I DON'T KNOW." em vez de "NO.".
- Contorno: depois de cada resposta, MODAL volta a NIL (`modal_zerado.lisp` embrulha a função ANSWER).

## Objetivos negados por último (GOALIZE2)

- Na 14, "i like blocks which are not red" vira o teorema: gosto de X se *não* (X é vermelho) e X é bloco. O "não" vem primeiro porque #COLOR tem prioridade 300 e #IS tem 100, e a GOALIZE2 dá ao objetivo negado a mesma prioridade do positivo.
- Com X ainda sem valor, o "não é vermelho" falha (existem coisas vermelhas). Assim a 16 só achava a pirâmide azul, que tinha sido declarada "nice" na 13.
- Testei: com X já amarrado ("do i like the blue block?"), o teorema responde YES.
- Correção: o objetivo negado recebe a prioridade com o sinal trocado, e fica depois dos objetivos que dão valor às variáveis. A 16 passa a achar os dois objetos da tese.
- Não achei prova direta do original para esta. É a correção mais simples que conserta a ordem.

## Os dois remendos na gramática compilada (frases 28 a 33)

- **Sintoma.** "why did you do that?" respondia "NO.". O "that" ficava sem significado, nenhuma relação era montada, e o programa caía na resposta de emergência, que imprime NO. → `transcricoes/rastreio_frases28a33.txt`
- **Causa.** No começo do sintagma nominal, a gramática de 1971 testa `(OR (NQ QDET) (NQ PRONREL))` para reconhecer pronomes interrogativos. THAT tem a marca PRONREL no dicionário (como WHO, WHAT e WHICH). Assim, todo "that" é lido como interrogativo, e a função SMTHAT, que liga "that" ao evento da última resposta, nunca roda. O mesmo teste aparece no objeto de verbo com partícula, e por isso "clear off that cube" falhava.
- **Evidência.** A versão posterior da gramática (its-vault, `grammar/gramar.61`) troca `PRONREL` por `QPRON` nesse ponto e apaga o segundo teste.
- **Remendo.** A gramática está compilada na imagem, então mudei o próprio código de máquina. Nos dois lugares, a instrução carregava a lista `(PRONREL)` (endereço 206614). Troquei pelo endereço 111674, uma lista `(REL)` que já existe na imagem. Só WHO tem a marca REL; o "that" demonstrativo deixa de entrar por esse caminho. São duas linhas: `(DEPOSIT 257064 201040111674)` e `(DEPOSIT 254030 201040111674)`.
- **Resultado.** As frases 28 a 33 saem palavra por palavra como na tese, inclusive a sequência longa da 32.

## A frase 23: SMIT3 e WHO corrigidas

- **O que a tese diz.** Depois do item 23, a tese explica: se a pergunta fosse "what size is it?", o "it" seria a pirâmide verde. Mas "ninguém perguntaria a cor de uma coisa que acabou de ser chamada de 'the green pyramid'", então o programa procura mais longe. A seção 1.1.2 (p. 27) dá a mesma regra: ninguém pergunta o que a frase anterior já respondeu.
- **Onde isso está no código.** Em ANS1 (`z.answer`, linhas 175–185). Quando há mais de uma interpretação, ANS1 refaz a busca só com os fatos já mencionados. Se a resposta sai igual usando só os fatos marcados na frase anterior, a interpretação perde 500 pontos. Se sai igual com tudo o que já foi dito, perde 200. Depois, o programa só escolhe sozinho quando a primeira passa a segunda por mais de 200 (TIMID). Se não passa, pergunta.
- **O que acontecia.** O "it" tinha três candidatos: o sujeito da frase 22 (300 pontos), "anything green" (200) e "the tallest pyramid's support" (100). Ninguém perdia pontos, e o programa perguntava qual "it". → `transcricoes/rastreio_frase23.txt`
- **Primeira falha: SMIT3.** Ela procura, na lista dos sintagmas da frase anterior, o que começa e termina no mesmo lugar que o sintagma em análise. A busca usa o fim como chave. "anything green" fecha a frase 22, e ali o fim vale NIL. Para qualquer sintagma com outro começo, a comparação também dá NIL, e a SMIT3 devolve o primeiro da lista. Foi assim que "the tallest pyramid's support" entrou no lugar de "anything green".
- **Segunda falha: WHO.** A busca "só com o que foi mencionado" troca o filtro do Micro-Planner por WHO, que só aceita fatos marcados com o número da frase em que apareceram. A palavra "color" sempre pede o fato `(#IS #GREEN #COLOR)`, "verde é uma cor". Esse fato vem do arquivo de dados e nunca recebe marca. Resultado: em pergunta de cor, a busca restrita sempre falha.
- **Correções.** `smit3_corrigida.lisp` compara o começo e o fim; se nada casar, devolve NIL. `who_corrigida.lisp` deixa passar, sem marca, os fatos que dizem quais palavras são cores ou formas (são oito no arquivo de dados). O resto da WHO fica igual. A primeira conserta um erro. A segunda é uma mudança minha: a versão reescrita (1973–74) filtra do mesmo jeito (MUMBLE, `smutil.152`), então não há prova de como o original tratava esse fato.
- **Testes a partir de um ponto de controle depois da frase 22.** Só com a SMIT3: "anything green" entra pelo caminho certo, com 300 (100, mais 200 por ser o foco da frase 22), empata com o sujeito, e o programa pergunta. Só com a WHO: os três candidatos ficam com 100, −100 e −300; a diferença entre os dois primeiros é 200, e o programa pergunta. Com as duas: "anything green" perde 500 e fica com −200; o sujeito perde 200 e fica com 100; a resposta sai como na tese.
- **Grafia.** Ao ler a frase, o programa troca o apóstrofo de dentro de uma palavra por aspas (`z.syscom`, linha 93) e imprime `PYRAMID"S`. A tese escreve `PYRAMID'S`.
- **As opções vazias.** Quando pergunta qual "it", o programa imprime uma propriedade que, para pronomes, ninguém grava no código de 1971. A versão seguinte (`smspec.89`, do its-vault) passa a gravar as palavras de cada candidato. Com as correções, a frase 23 não pergunta mais.

## A frase 35: a lista de teoremas de #LOC

- **Sintoma.** "there were five blocks to the left of the box then." respondia "NO, ONLY, NONE OF THEM.". O programa achava zero blocos à esquerda da caixa. → `transcricoes/rastreio_frase35.txt`
- **O que já funcionava.** O "then" copia o tempo da frase 34 ("before you started to clean it off"), e a pergunta passa a valer para aquele instante.
- **Causa.** Posição relativa ("à esquerda de", "atrás de") nunca fica guardada: é sempre calculada por um teorema. O teorema vem da propriedade THMLIST da relação, escolhido pelo número de elementos do objetivo. Com tempo, o objetivo #LOC tem 5 elementos, mas a THMLIST de #LOC só tinha a entrada de 4. Sem teorema, o Micro-Planner só procura fatos guardados, e não há nenhum fato #LOC.
- **Por que a entrada sumiu.** Em `z.dictio`, a lista de THMLIST tem #LOC duas vezes. A linha 892 traz as entradas de 4 e de 5 elementos; a de 5 usa o teorema com tempo, TCT-LOC. A linha 913 traz só a de 4. DEFLIST grava as linhas em ordem, e a segunda apaga a primeira. O teorema TCT-LOC existe em `z.blocks`, mas nada mais o chama.
- **Evidência.** As versões seguintes (`dictio.70` e `dictio.71` do its-vault, e `dictio.76` da versão reescrita) guardam as duas entradas.
- **Correção.** `loc_thmlist_corrigida.lisp` grava em #LOC o valor da linha 892. A correção é minha, mas o valor é do próprio arquivo.
- **Resultado.** A busca acha quatro blocos: o cubo vermelho, os dois cubos verdes grandes e o bloco vermelho grande, como na tese. O programa escreve "NO, ONLY, FOUR OF THEM", com uma vírgula depois de ONLY (`z.answer`, linha 214). A tese escreve "NO, ONLY FOUR OF THEM".

## A frase 37: três correções

- **Sintoma.** "is there anything which is bigger than every pyramid but is not as wide as the thing that supports it?" dava `CAR MADE ILGL MEM REF`. → `transcricoes/rastreio_frase37.txt`
- **Primeira falha: SMRSQ.** A frase liga duas orações relativas com "but". A SMCONJ guarda as duas como `(BUT …)`, e a SMRSQ só reconhece AND. A versão seguinte (`smspec.88` e `smspec.89`, linha 48) trata BUT igual a AND. Correção: a SMRSQ aceita os dois. Só esse teste muda.
- **Segunda falha: "anything" não gera candidatos.** Com a SMRSQ corrigida, o Micro-Planner parava: a comparação "bigger than" recebia o objeto sem valor. Na busca, o único objetivo fora de um "não" é "existe X", e o teorema dele aceita sem dar valor a X. Em 1971, "anything" recebe só uma marca (#VAGUE) e nenhum objetivo. As versões seguintes (`smspec.88`, `smspec.89` e `smspec.97`) dão a ele o objetivo `(#PHYSOB X)`, "X é um objeto físico". Correção: o mesmo objetivo, na SMNG1 de 1971.
- **Terceira falha: SIZE não conhece a mão.** O objetivo `(#PHYSOB X)` também gera a mão do robô, e a comparação de tamanho dava erro com ela. A versão reescrita (`blockl.7`, linha 294) acrescenta a mão à SIZE, com tamanho zero. Correção: a mesma linha.
- **Resultado.** A frase roda até o fim e responde "YES, FOUR OF THEM: THE TABLE, THE BOX, A LARGE RED BLOCK AND THE BLUE BLOCK." A tese responde só "YES, THE BLUE BLOCK."
- **Por que quatro.** Pelos tamanhos que o programa usa (o arquivo de dados e a função SIZE), a mesa, a caixa e o bloco vermelho grande também são maiores que todas as pirâmides. E cada um é mais estreito do que a coisa que o apoia (a mesa não tem apoio). Pelas regras do programa, os quatro servem. Não achei, no código nem na tese, o que deixaria só o bloco azul. Conto a 37 como quase igual, como a 19.
- **Um erro que fica.** O teorema de "as wide as" (TC-ASMUCH) nunca funciona: no padrão está escrito MEASURE, onde o teorema gêmeo de "bigger than" (TC-MORE) usa a variável `$?MEASURE`. Testei a troca (`asmuch_teste_37.lisp`): a resposta da 37 não muda, porque nenhum dos quatro é tão largo quanto o que o apoia. O erro existe também na versão reescrita, então não mexi.

## Frase a frase (rodada 15)

| nº | frase | tese (1971) | moby dump, rodada 15 | | obs. |
|---|---|---|---|---|---|
| 1 | pick up a big red block. | OK. | OK. | = |  |
| 2 | grasp the pyramid. | I DON'T UNDERSTAND WHICH PYRAMID YOU MEAN. | I DON'T KNOW WHICH PYRAMID YOU MEAN. | ≈ | o próprio z.answer diz "I DON'T KNOW WHICH"; a tese imprimiu outra frase |
| 3 | find a block which is taller than the one you are holding and put it into the box. | BY "IT", I ASSUME YOU MEAN THE BLOCK WHICH IS TALLER THAN THE ONE I AM HOLDING. OK. | OK. BY "IT", I ASSUME YOU MEAN THE BLOCK WHICH IS TALLER THAN THE ONE I AM HOLDING. OK. | = | sai um "OK." a mais antes |
| 4 | what does the box contain? | THE BLUE PYRAMID AND THE BLUE BLOCK. | THE BLUE PYRAMID AND THE BLUE BLOCK. | = |  |
| 5 | what is the pyramid supported by? | THE BOX. | THE BOX. | = | certa só com a FINDQ reconstruída |
| 6 | how many blocks are not in the box? | FOUR OF THEM. | FOUR OF THEM. | = |  |
| 7 | is at least one of them narrower than the one which i told you to pick up? | YES, THE RED CUBE. | BY "THEM", I ASSUME YOU MEAN THE HOW MANY BLOCKS AM NOT IN THE BOX. YES, THE RED CUBE. | = | sai antes: BY "THEM", I ASSUME YOU MEAN THE HOW MANY BLOCKS AM NOT IN THE BOX. |
| 8 | is it supported? | YES, BY THE TABLE. | BY "IT", I ASSUME YOU MEAN THE AT LEAST ONE OF THEM NARROWER THAN THE ONE WHICH YOU TOLD I TO PICK UP. YES, BY THE TABLE. | = | sai antes: BY "IT", I ASSUME YOU MEAN THE AT LEAST ONE OF THEM NARROWER ... |
| 9 | can the table pick up blocks? | NO. | NO. | = |  |
| 10 | can a pyramid be supported by a block? | YES. | YES. | = |  |
| 11 | can a pyramid support a pyramid? | I DON'T KNOW. | I DON'T KNOW. | = |  |
| 12 | stack up two pyramids. | I CAN'T. | I CAN'T. | = |  |
| 13 | the blue pyramid is nice. | I UNDERSTAND. | I UNDERSTAND. | = | certa com a SMADJ corrigida |
| 14 | i like blocks which are not red, but i don't like anything which supports a pyramid. | I UNDERSTAND. | I UNDERSTAND. I UNDERSTAND. | = | sai I UNDERSTAND. duas vezes, uma para cada metade da frase |
| 15 | do i like the box? | NO. | NO. | = | certa com MODAL zerado depois de cada resposta |
| 16 | do i like anything in the box? | YES, TWO THINGS: THE BLUE BLOCK AND THE BLUE PYRAMID. | YES, TWO OF THEM: THE BLUE BLOCK AND THE BLUE PYRAMID | ≈ | diz OF THEM em vez de THINGS; acha os dois objetos com a ordem dos negados corrigida |
| 17 | will you please stack up both of the red blocks and either a green cube or a pyramid? | OK. | OK. | = |  |
| 18 | which cube is sitting on the table? | THE LARGE GREEN ONE WHICH SUPPORTS THE RED PYRAMID. | THE LARGE GREEN ONE WHICH SUPPORTS THE RED PYRAMID. | = |  |
| 19 | is there a large block behind a pyramid? | YES, THREE OF THEM: A LARGE RED ONE, A LARGE GREEN CUBE AND THE BLUE ONE. | YES, TWO OF THEM: A LARGE RED ONE AND THE BLUE ONE. | ≈ |  |
| 20 | put a small one onto the green cube which supports a pyramid. | OK. | OK. | = |  |
| 21 | put the littlest pyramid on top of it. | OK. | OK. | = |  |
| 22 | does the shortest thing the tallest pyramid's support supports support anything green? | YES, THE GREEN PYRAMID. | YES, THE GREEN PYRAMID. | = |  |
| 23 | what color is it? | BY "IT", I ASSUME YOU MEAN THE SHORTEST THING THE TALLEST PYRAMID'S SUPPORT SUPPORTS. RED | BY "IT", I ASSUME YOU MEAN THE SHORTEST THING THE TALLEST PYRAMID"S SUPPORT SUPPORTS. RED | = | certa com SMIT3 e WHO corrigidas; o programa escreve PYRAMID"S |
| 24 | how many things are on top of green cubes? | I'M NOT SURE WHAT YOU MEAN BY "ON TOP OF" IN THE PHRASE "ON TOP OF GREEN CUBES". DO YOU MEAN: 1 - DIRECTLY ON THE SURFACE 2 - ANWHERE ON TOP OF ? [2] THREE OF THEM. | THREE OF THEM | ≈ | a pergunta de esclarecimento não saiu; na rodada 7 saiu igual à tese e o "2" foi aceito |
| 25 | had you touched any pyramid before you put the green one on the little cube? | YES, THE GREEN ONE. | YES, THE GREEN ONE. | = |  |
| 26 | when did you pick it up? | WHILE I WAS STACKING UP THE RED CUBE, A LARGE RED BLOCK AND A LARGE GREEN CUBE. | BY "IT", I ASSUME YOU MEAN THE PYRAMID BEFORE I PUT THE GREEN ONE ON THE LITTLE CUBE. WHILE I WAS STACKING UP THE RED CUBE , A LARGE RED BLOCK AND A LARGE GREEN CUBE. | = | sai antes: BY "IT", I ASSUME YOU MEAN THE PYRAMID BEFORE I PUT THE GREEN ONE ON THE LITTLE CUBE. |
| 27 | why? | TO GET RID OF IT. | TO GET RID OF IT. | = |  |
| 28 | why did you do that? | TO CLEAN OFF THE RED CUBE. | TO CLEAN OFF THE RED CUBE. | = | certa com os remendos da gramática |
| 29 | why did you clear off that cube? | TO PUT IT ON A LARGE GREEN CUBE. | TO PUT IT ON A LARGE GREEN CUBE. | = | certa com os remendos da gramática |
| 30 | why did you do that? | TO STACK UP THE RED CUBE , A LARGE RED BLOCK AND A LARGE GREEN CUBE. | TO STACK UP THE RED CUBE , A LARGE RED BLOCK AND A LARGE GREEN CUBE. | = | certa com os remendos da gramática |
| 31 | why did you do that? | BECAUSE YOU ASKED ME TO. | BECAUSE YOU ASKED ME TO. | = | certa com os remendos da gramática |
| 32 | how did you do it? | BY PUTTING A LARGE RED BLOCK ON THE TABLE ; THEN LETTING GO OF IT ; THEN PUTTING A LARGE GREEN CUBE ON IT ; THEN LETTING GO OF THAT CUBE ; THEN PUTTING THE RED CUBE ON THAT CUBE ; THEN LETTING GO OF THAT CUBE. | BY PUTTING A LARGE RED BLOCK ON THE TABLE ; THEN LETTING GO OF IT ; THEN PUTTING A LARGE GREEN CUBE ON IT ; THEN LETTING GO OF THAT CUBE ; THEN PUTTING THE RED CUBE ON THAT CUBE ; THEN LETTING GO OF THAT CUBE. | = | certa com os remendos da gramática |
| 33 | how many objects did you touch while you were doing it? | FOUR OF THEM. | FOUR OF THEM. | = | certa com os remendos da gramática |
| 34 | what did the red cube support before you started to clean it off? | THE GREEN PYRAMID. | THE GREEN PYRAMID. | = |  |
| 35 | there were five blocks to the left of the box then. | NO, ONLY FOUR OF THEM: THE RED CUBE , TWO LARGE GREEN CUBES AND A LARGE RED BLOCK. | NO, ONLY, FOUR OF THEM: THE RED CUBE , TWO LARGE GREEN CUBES AND A LARGE RED BLOCK. | = | certa com a THMLIST de #LOC; sai uma vírgula depois de ONLY |
| 36 | put the blue pyramid on the block in the box. | OK. | OK. | = |  |
| 37 | is there anything which is bigger than every pyramid but is not as wide as the thing that supports it? | YES, THE BLUE BLOCK. | YES, FOUR OF THEM: THE TABLE , THE BOX , A LARGE RED BLOCK AND THE BLUE BLOCK. | ≈ | roda com SMRSQ, SMNG1 e SIZE corrigidas; acha também a mesa, a caixa e o bloco vermelho grande |
| 39 | does a steeple | SORRY, I DON'T KNOW THE WORD "STEEPLE". PLEASE TYPE TWO SPACES. | SORRY, I DON'T KNOW THE WORD "STEEPLE". PLEASE TYPE TWO SPACES. | = |  |
| 40 | a "steeple" is a stack which contains two green cubes and a pyramid. | I UNDERSTAND. | I UNDERSTAND. | = |  |
| 41 | are there any steeples now? | NO. | NO. | = |  |
| 42 | build one. | OK. | OK. | = | certa com a SMONE corrigida |
| 43 | call the biggest block "superblock". | OK. | OK. | = | certa com a SMPROP corrigida |
| 44 | have you picked up superblock since we began? | YES. | YES. | = | certa com a SMPROP corrigida |
| 45 | why did you drop it? | BY "IT", I ASSUME YOU MEAN THE SUPERBLOCK. I CAN'T EXPLAIN A NON-EXISTENT EVENT. | BY "IT", I ASSUME YOU MEAN THE SUPERBLOCK. I CAN'T EXPLAIN A NON-EXISTENT EVENT. | = | certa com a SMPROP corrigida |
| 46 | is there anything to the right of the red pyramid? | YES, FOUR THINGS: THE BOX , THE RED CUBE, THE BLUE PYRAMID AND THE BLOCK NAMED SUPERBLOCK | YES, FOUR OF THEM: THE BOX , THE RED CUBE , THE BLUE PYRAMID AND THE BLUE BLOCK. | ≈ | diz THE BLUE BLOCK em vez de THE BLOCK NAMED SUPERBLOCK, e OF THEM em vez de THINGS |
| 47 | thank you. | YOU'RE WELCOME! | YOU'RE WELCOME! | = |  |

## O que ainda difere da tese

Nenhuma frase está errada. Seis saem quase iguais.

**Quase iguais.** A 2 é o próprio código Z ("I DON'T KNOW WHICH"). A 16 diz "TWO OF THEM" em vez de "TWO THINGS" (ver abaixo). A 19 lista dois blocos em vez de três. A 24 não fez a pergunta de esclarecimento nesta rodada (na rodada 7 fez, igual à tese, e aceitou o "2"). A 46 lista as quatro coisas certas, mas diz "THE BLUE BLOCK" em vez de "THE BLOCK NAMED SUPERBLOCK" e "FOUR OF THEM" em vez de "FOUR THINGS". A 37 acha o bloco azul e mais três objetos (ver acima).

**"OF THEM" ou "THINGS".** A tese usa "THINGS" nas frases 16 e 46 e "OF THEM" nas 24 e 33. No código (ANS2, em `z.answer`), a escolha depende da marca #VAGUE do sintagma perguntado. A linha que lê essa marca usa `(CAR …)` onde a linha que grava LASTREL, na mesma função, usa `(CAAR …)`. Assim a marca nunca é achada, e sai sempre "OF THEM". Corrigir essa linha daria "THINGS" na 24 e na 33, porque "thing" e "object" têm #VAGUE no dicionário (`z.dictio`, linhas 515 e 606). Por isso não mexi.

## Comparação com a versão reescrita (1973–74) rodando em 2024

A sessão de Eric Swenson (pasta `05_transcricao_ITS_2024/`) usa o código reescrito. Ela não segue o roteiro inteiro, então só dá para comparar algumas frases:

| nº | versão reescrita, 2024 | moby dump, rodada 15 | tese |
|---|---|---|---|
| 4 | THE BLUE BLOCK AND THE BLUE PYRAMID. | THE BLUE PYRAMID AND THE BLUE BLOCK. | THE BLUE PYRAMID AND THE BLUE BLOCK. |
| 5 | THE BOX. | THE BOX. (só com a FINDQ reconstruída) | THE BOX. |
| 7 | sem resposta; depois I DON'T UNDERSTAND. | YES, THE RED CUBE. | YES, THE RED CUBE. |
| 9 | THAT DOESN'T MAKE ANY SENSE TO ME. | NO. | NO. |
| 17 | I DON'T KNOW WHAT YOU MEAN BY "BOTH OF THE RED BLOCKS…" | OK. | OK. |
| 22 | I DON'T UNDERSTAND. | YES, THE GREEN PYRAMID. | YES, THE GREEN PYRAMID. |
| 27 | SMADJQSHORT NOT WRITTEN YET | TO GET RID OF IT. | TO GET RID OF IT. |
| 28 | I CAN'T DISCUSS A NON-EXISTENT EVENT. | TO CLEAN OFF THE RED CUBE. | TO CLEAN OFF THE RED CUBE. |
| 33 | I DON'T KNOW WHAT "IT" REFERS TO | FOUR OF THEM. | FOUR OF THEM. |

## Como achei as falhas novas

- **Ponto de controle.** Rodei o diálogo até a frase anterior à falha, dei Ctrl-G e salvei o job inteiro com `:pdump oshrdl;s41 dump` (no DDT). Cada teste recarrega esse arquivo em segundos, em vez de repetir 40 frases.
- **Atenção ao Ctrl-G.** Depois de responder, o programa fica esperando uma tecla antes de fechar a frase. Um Ctrl-G nessa hora deixa a frase pela metade (`LASTSENT` continua apontando para a frase anterior). Por isso o roteiro manda um espaço antes do Ctrl-G.
- **Voltar ao diálogo.** `(SHRDLU)` recomeça a conversa do zero: chama FORGET e zera `LASTSENT`. Para continuar, uso `SHRDLU2` (`shrdlu2.lisp`), a mesma função copiada de `z.syscom`, que só pula essas duas linhas de entrada.
- **Rastreio.** `trace_defs.lisp` define `TR`, que embrulha uma função interpretada e imprime `>NOME` na entrada e `<NOME` na saída. Quando o erro sai, as entradas sem saída mostram onde ele estava.
- **Rastreio com valores.** `trace_valores.lisp` define `TRV`, que imprime também os argumentos e o valor de cada chamada. Foi assim que apareceram os pontos de cada candidato de "it" na frase 23.

## Histórico das rodadas

| rodada | arquivo em `transcricoes/` | configuração |
|---|---|---|
| 1 | `rodada1_imagem_como_veio.txt` | imagem como veio |
| 2 | `rodada2_memoria_ligada.txt` | + `NOMEM NIL` |
| 3 | `rodada3_VALUE_NIL.txt` | + `VALUE NIL` |
| 4 | `rodada4_VALUE_NIL_sem_7_e_8.txt` | idem, pulando 7 e 8 |
| 6 | `rodada6_SMNGOF_Z.txt` | + SMNGOF dos arquivos Z (para na 23: o "2" não era aceito) |
| 7 | `rodada7_READCH_v1.txt` | + primeira versão do ajuste do READCH (tinha um defeito: engolia o resto da linha depois de um apóstrofo) |
| 8 | `rodada8.txt` | + READCH corrigido |
| 9 | `rodada9_definitiva.txt` | + FINDQ reconstruída, disco do ITS refeito do zero |
| 10 | `rodada10_definitiva.txt` | + SMONE e SMPROP corrigidas |
| 11 | `rodada11_definitiva.txt` | + SMADJ corrigida, MODAL zerado, negados por último |
| 12 | `rodada12_definitiva.txt` | + dois remendos na gramática compilada |
| 13 | `rodada13_definitiva.txt` | + SMIT3 e WHO corrigidas |
| 14 | `rodada14_definitiva.txt` | + lista de teoremas de #LOC |
| 15 | `rodada15_definitiva.txt` | + SMRSQ, SMNG1 e SIZE corrigidas |

A rodada 5 foi um ensaio da 6, com a mesma configuração. Os testes a partir dos pontos de controle estão em `teste_a_partir_da_41.txt`, `teste_a_partir_da_42.txt`, `teste_frases13a16.txt`, `teste_frases28a33.txt`, `rastreio_frase23.txt`, `rastreio_frase35.txt` e `rastreio_frase37.txt`. Os consoles brutos do ITS estão em `console_ITS.log` (rodadas 1–4) e `console_ITS_rodadas6a15.log`.

## Rodada 16 (13/10/2026): 36 idênticas, 5 com texto a mais, 5 diferentes

Contagem estrita, para publicação (comparação ignora só maiúsculas e pontuação):
- **36 idênticas** palavra por palavra à tese.
- **5 com a resposta da tese mais uma frase que a tese não tem** (3, 7, 8, 14, 26). Na 3 sobra um "OK." no início; na 14 o "I UNDERSTAND." sai duas vezes; na 7, 8 e 26 sobra uma frase "BY ..., I ASSUME YOU MEAN ..." com inglês quebrado ("THE HOW MANY BLOCKS AM NOT IN THE BOX", "WHICH YOU TOLD I TO PICK UP"). São defeitos visíveis.
- **5 diferentes** (2, 19, 24, 37, 46): 2 muda a palavra ("KNOW" por "UNDERSTAND", como está no z.answer); 19 lista dois objetos em vez de três; 24 não faz a pergunta de esclarecimento; 37 lista quatro objetos em vez de um; 46 chama o bloco azul de "THE BLUE BLOCK" em vez de "THE BLOCK NAMED SUPERBLOCK".
- Nas rodadas anteriores eu somava as 5 "com texto a mais" às iguais ("41 iguais"). Para publicar, use a contagem estrita acima.

### Inventário completo do que é digitado por cima da imagem original (rodada 16)
Nada é alterado nos arquivos originais; tudo é redefinido na hora de rodar, por `rodada16_completa.sh`.
1. Quatro variáveis: `NOSTOP T`, `NOMEM NIL`, `VALUE NIL`, `PLANNERSEE NIL`.
2. Código do próprio Winograd, tirado dos arquivos Z: `SMNGOF` (1).
3. Contornos do ambiente MacLisp/ITS: `READCH` e um invólucro de `PUTPROP` (2).
4. Função que não existe em lugar nenhum, reconstruída por mim: `FINDQ` (1).
5. Correções de erros do código da época (restauração): `SMONE`, `SMPROP`, `SMADJ`, `SMIT3`, THMLIST de `#LOC`, `ANS2` parte 1 (6).
6. Escolhas minhas com justificativa (reconstrução): `WHO`, MODAL zerado, `GOALIZE2`, `SMRSQ`, `SMNG1`, `SIZE`, `ANS2` parte 2, e o remendo na gramática compilada (`remendo_ng.lisp`: duas palavras de código de máquina alteradas, contadas como uma intervenção) (8).
Total: 18 intervenções de código mais 4 variáveis. Cada uma tem a explicação nas seções acima.

Sessão nova, as 46 frases, as correções da rodada 15 mais `ans2_corrigida.lisp`. Transcrição: `transcricoes/rodada16_definitiva.txt`; tabela: `tabela_rodada16.tsv`.

**ANS2: "THINGS" ou "OF THEM".** Duas mudanças na ANS2 (`z.answer`):
1. Restauração: a linha que lê a marca #VAGUE usa `(CAAR (CADDDR ANS))`, como a linha que grava LASTREL na mesma função (antes era `CAR`, e a marca nunca era achada).
2. Reconstrução pela tese (seção 4.4.2, p. 388): perguntas HOW MANY respondem sempre "OF THEM". Por isso o teste HOWMANY vem antes do teste VAGUE. A versão de 1972 (NAMESUGAR, `newans.78`) lê a mesma marca #VAGUE.

Efeito: a 16 fica igual à tese ("YES, TWO THINGS: ..."); a 46 passa a dizer "FOUR THINGS"; a 24 e a 33 continuam "OF THEM", como na tese. Nenhuma frase piorou.

**NOSTOP, para quem for conferir.** No código da época, NOSTOP só é consultado em ERT (`z.syscom`, linha 50: `(AND NOSTOP (ERR NIL))`, isto é, com NOSTOP T um ponto de parada vira erro e volta ao nível de cima em vez de esperar o teclado), em ERTSTOP (que liga NOSTOP NIL e para sempre), no caminho de "I DON'T UNDERSTAND" (`z.syscom`, linha 571) e em THVAL2 quando PLANNERSEE está ligado (`z.semant`, linha 778). A pergunta de esclarecimento da frase 24 ("DO YOU MEAN: 1 - ... 2 - ...") é feita por YELL (`z.answer`, linha 725) e não depende de NOSTOP: ela só sai quando a frase tem mais de uma interpretação e a primeira não vence a segunda por mais de TIMID. Pelo código, a única forma de a pergunta não sair é a frase 24 chegar a YELL com uma interpretação só, ou com uma vencedora folgada; a causa está antes, na análise semântica de "on top of", e não foi investigada. A transcrição de 2024 da versão reescrita, com NOSTOP NIL, também responde a 24 sem a pergunta.

**Frase 46: por que continua "THE BLUE BLOCK".** A base mostra `(#CALL :B6 SUPERBLOCK)`: o programa dá o nome ao bloco vermelho grande. "Biggest" usa #SIZE, que na imagem é `(LAMBDA (X) (APPLY 'PLUS (SIZE X)))`, a soma das dimensões (a tese diz o mesmo, seção 3.4.2). Com os tamanhos do arquivo de dados (iguais em 1971, 1972 e 2024), B6 (200 300 300) soma 1000 e B10 (200 100 400) soma 700, em octal. O B10 só seria o "superblock" com outros dados; não mexi.

**Quase iguais agora:** 2, 19, 24 (sem a pergunta de esclarecimento), 37 e 46.

**Fonte nova:** os fontes do Micro-Planner de 1971 (plnr.169–171, set./out. 1971; plnr.176, dez. 1971) estão em <https://github.com/MITDDC/plnr-1971-1982> (fitas do MIT, coleção MC-0741). Até aqui o Micro-Planner da época só existia compilado dentro do moby dump.
