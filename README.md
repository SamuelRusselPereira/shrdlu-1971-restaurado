# SHRDLU da época da tese (1970–71): decodificado e rodando de novo

A versão do SHRDLU da época da tese de Terry Winograd (1970–71) está em uma imagem de memória (`moby.dump`) e oito arquivos-fonte (os "arquivos Z") guardados em uma fita do MIT e públicos no repositório its-vault desde 2018, sem que, até onde encontrei, alguém a tivesse identificado como a versão da tese, decodificado ou executado. Decodificada e executada em um PDP-10 emulado, com 17 intervenções de código e 4 variáveis de ambiente, todas documentadas, por cima dos arquivos originais intactos, ela reproduz **36 das 46 respostas do diálogo da tese palavra por palavra**; outras 5 saem com a resposta da tese mais uma frase a mais, e 5 saem diferentes. Só com o código do próprio Winograd e os contornos de ambiente, sem nenhuma correção, são 20 (tabela cumulativa abaixo).

Trabalho feito por Samuel Russel Pereira com o Claude (Anthropic), de 10 a 13 de outubro de 2026. Tudo o que está aqui veio de fontes públicas (links no fim). Nenhum arquivo original foi editado: cada ajuste é um arquivo separado, carregado na hora de rodar.

## Summary in English

The thesis-era version of SHRDLU (1970–71) survives in the old `SHRDLU` directory of the MIT AI Lab tape `tid3100015`, published in the **PDP-10/its-vault** repository in 2018 and, as far as I could find, never identified as the thesis version, decoded or run since: a MacLisp memory image (`moby.dump`, 402,480 bytes; the directory index says *"it can execute the entire dialog"*) and eight source files `z.answer`, `z.blocks`, `z.break`, `z.dictio`, `z.gramar`, `z.mover`, `z.semant`, `z.syscom` (*"files used by the original implementation of SHRDLU"*). The directory's second index, `#files.6`, written by the MIT group after 1972, dates them: *"the files whose first names are "Z" are the original files of the system circa January 1971"* (the thesis is dated February 1971). The thesis appendices (AI-TR-235) were printed from these files: the dictionary entry BIG is identical to Appendix E, the noun-group grammar has the same labels and notation as Appendix D, and the theorem TCT-PICKUP carries the same typo (`#PICKUP$?EV`, no space) in the file and on the printed page (page crops next to the file lines: `06_verificacoes/tese_paginas/`). The image was decoded (234 of 238 comparable interpreted definitions are identical to the Z files) and found in the initial state of the thesis dialogue (the 73 Micro-Planner assertions of Appendix F, checked one by one). Loaded into ITS on the KA10 emulator of the PDP-10/its project, with 4 variables and 17 documented code interventions typed over the untouched originals, it reproduces **36 of the 46 answers of the thesis dialogue word for word**; 5 more give the thesis answer plus an extra sentence, and 5 differ. With Winograd's own code and the environment workarounds alone, no fixes, 20 answers are already identical; the cumulative table below separates what comes from 1971 from what comes from 2026. Every intervention, every transcript and every comparison is in `10_moby_rodando/` (in English: `HOW_TO_RUN.md` and `RESULTS_EN.md`, the sentence-by-sentence table). No public record was found of anyone having identified these files as the thesis version, decoded the image or run it before.

## Descoberta principal: o SHRDLU original está no its-vault

O repositório **PDP-10/its-vault** guarda uma cópia mais antiga do diretório SHRDLU do MIT (fita `tid3100015`, importada por Lars Brinkhoff em 2018). Nela estão duas coisas que os índices do próprio diretório (`#files.info`, `#files.6`) descrevem. Os arquivos estão públicos desde 2018 e os índices sempre disseram o que são (o `#files.6` chega a datar os arquivos Z: "circa January 1971"); o que não encontrei foi registro de alguém que, desde então, os tivesse conferido contra a tese, decodificado a imagem ou posto o diálogo para rodar:

- **MOBY DUMP** (`moby.dump`, 402.480 bytes): imagem executável do MacLisp com o SHRDLU carregado. O índice diz: *"an ancient vintage core dump used for occasional demos. Its only redeming feature is that it can execute the entire dialog"*.
- **Arquivos Z** (`z.answer`, `z.blocks`, `z.break`, `z.dictio`, `z.gramar`, `z.mover`, `z.semant`, `z.syscom`; 8.339 linhas): *"files used by the original implementation of SHRDLU (there are known bugs in the grammar)"*.

O que foi conferido:

1. **Os arquivos Z são da época da tese (1970–71).** O segundo índice do diretório, `#files.6` (escrito pelo grupo do MIT depois de 1972: cita o PLNR 180, o livro de Winograd e "Terry Winograd (currently at Stanford University)"), diz: *"the files whose first names are "Z" are the original files of the system circa January 1971. They use the old list oriented data structures and are essentially uncommented"*. A tese (AI-TR-235) é de fevereiro de 1971. Os arquivos em si não trazem data (o mesmo diretório tem arquivos posteriores, como o `dictio.71`, com um comentário de junho de 1972), e o conteúdo bate com a datação do índice: tudo indica um retrato de pouco antes da versão que gerou o diálogo impresso (ver "Pistas do que ainda falta"). As datas de criação dos arquivos na fita `tid3100015`, se existirem, fechariam a questão. Os apêndices da tese (AI-TR-235) foram impressos a partir deles. A entrada BIG do dicionário é idêntica ao Apêndice E. A gramática do sintagma nominal tem os mesmos rótulos e a mesma notação do Apêndice D. O teorema TCT-PICKUP tem o mesmo erro de digitação (`#PICKUP$?EV`, sem espaço) no arquivo e na página impressa. O dicionário tem NICE e LIKE, as palavras do diálogo da tese. Recortes das páginas 451 e 455 da tese ao lado das linhas dos arquivos Z: `06_verificacoes/tese_paginas/`.
2. **O moby dump foi gerado a partir dos arquivos Z.** A imagem foi decodificada e as definições guardadas nela foram reimpressas. Seis arquivos Z (`answer`, `blocks`, `break`, `dictio`, `semant`, `syscom`) têm 292 definições de topo; 238 delas têm texto interpretado na imagem (178 funções, 56 teoremas, 4 valores) e, dessas, 234 são idênticas ao texto dos arquivos Z. Das 4 diferentes, só uma é código: a função SMNGOF ("one of them"); as outras são um caractere de fim de linha, o erro de digitação de TCT-PICKUP (que o MacLisp lê igual) e a ERT redefinida para o ambiente de demonstração. As outras 54 (a gramática, os verbos de movimento, parte do `syscom`) estão compiladas na imagem; o fonte está nos arquivos Z. → `09_moby_dump_extraido/comparacao_arquivos_Z_x_moby.txt`
3. **A imagem está no estado inicial do diálogo.** O banco do Micro-Planner tem as 73 asserções do Apêndice F da tese, conferidas uma a uma, mais `(#START EE :DIALOG)`. A tabela do mundo (ATABLE) é igual à do arquivo `data`. Detalhe: dentro da própria imagem, a posição `#AT` de B1 e B2 é x = 100 (como na tese), mas a ATABLE diz 110. O braço do demo segue a ATABLE.
4. **Os arquivos Z não têm os avisos "NOT WRITTEN YET" da versão reescrita.** Nenhuma função é um aviso no lugar de código, e LASTEVENT é preenchido (`z.semant`, linha 71). Duas funções, porém, são chamadas e não estão definidas em lugar nenhum: SMPLACE e FINDQ (ver "Pistas do que ainda falta").

→ `07_its_vault_SHRDLU_original/`, `09_moby_dump_extraido/`

## O SHRDLU da época da tese voltou a rodar

O moby dump foi carregado no ITS, dentro do emulador KA10 do projeto PDP-10/its, e respondeu ao diálogo inteiro da tese (46 frases, digitadas na ordem da tese por `dialogo2.py`, em sessão nova com a imagem recém-carregada).

**Resultado da rodada 16 (13/10/2026), contagem estrita** (a comparação ignora só maiúsculas e pontuação):

| | Frases |
|---|---|
| **36 idênticas** à tese, palavra por palavra | todas as outras |
| **5 com a resposta da tese mais uma frase que a tese não tem** | 3, 7, 8, 14, 26 |
| **5 diferentes** | 2, 19, 24, 37, 46 |

Nas 5 "com texto a mais": na 3 sobra um "OK." no início; na 14 o "I UNDERSTAND." sai duas vezes; na 7, 8 e 26 sobra uma frase "BY ..., I ASSUME YOU MEAN ..." com inglês quebrado. São defeitos visíveis. Nas 5 diferentes: a 2 muda uma palavra ("KNOW" por "UNDERSTAND", como está no `z.answer`); a 19 lista dois objetos em vez de três; a 24 não faz a pergunta de esclarecimento; a 37 lista quatro objetos em vez de um; a 46 chama o bloco azul de "THE BLUE BLOCK" em vez de "THE BLOCK NAMED SUPERBLOCK" (pelos dados do próprio programa, o "superblock" é o B6, e não o B10 da tese; o tamanho é a soma das dimensões, como diz a seção 3.4.2 da tese).

**Tabela cumulativa: quanto vem de 1971 e quanto vem de 2026.** Três rodadas a mais (13/10/2026), cada uma em sessão nova, com subconjuntos crescentes dos ajustes:

| Rodada | O que está carregado | Idênticas | Com texto a mais | Diferentes |
|---|---|---|---|---|
| A | só o original + ambiente: 4 variáveis, `READCH`, `SMNGOF` dos arquivos Z | **20** | 4 | 22 |
| B | A + `FINDQ` reconstruída (2 linhas) | **21** | 4 | 21 |
| C | B + 6 correções de erros do código da época | **27** | 5 | 14 |
| 16 | C + 8 escolhas de reconstrução | **36** | 5 | 5 |

A FINDQ muda só a frase 5. As 6 correções acrescentam 13, 35 e 42 a 45 (e levam a 14 de errada para "texto a mais"). As 8 escolhas acrescentam 15, 16, 23 e 28 a 33; destas, 28 a 33 dependem do remendo na gramática compilada e das mudanças em SMIT3 e WHO. Transcrições e tabelas: `10_moby_rodando/transcricoes/cumulativa_*.txt`, `10_moby_rodando/tabela_cumulativa_*.tsv`; script: `rodadas_cumulativas.sh`.

**O que é digitado por cima da imagem original** (nada é alterado nos arquivos; tudo é redefinido na hora de rodar, por `rodada16_completa.sh`):

1. Quatro variáveis: `NOSTOP T`, `NOMEM NIL`, `VALUE NIL`, `PLANNERSEE NIL`.
2. Código do próprio Winograd, tirado dos arquivos Z: `SMNGOF` (1).
3. Contorno do ambiente MacLisp/ITS: `READCH` redefinida por cima da original (duas linhas: a primeira guarda a READCH original como OLDREADCH, a segunda define a nova, que converte o dígito lido em número) (1).
4. Função que não existe em lugar nenhum, reconstruída: `FINDQ`, duas linhas (1).
5. Correções de erros do código da época (restauração): `SMONE`, `SMPROP`, `SMADJ`, `SMIT3`, lista de teoremas de `#LOC`, `ANS2` parte 1 (6).
6. Escolhas com justificativa (reconstrução): `WHO`, MODAL zerado, `GOALIZE2`, `SMRSQ`, `SMNG1`, `SIZE`, `ANS2` parte 2, e o remendo na gramática compilada (duas palavras de código de máquina alteradas, contadas como uma intervenção: um arquivo, `remendo_ng.lisp`) (8).

Total: 17 intervenções de código mais 4 variáveis. Cada uma está explicada, com o motivo e o efeito frase a frase, em `10_moby_rodando/RESULTADOS.md`.

Outros pontos conferidos:

- A trava do Micro-Planner (`THUNBOUND THGAL` na frase 7) vinha de uma SMNGOF mais antiga, guardada na imagem. Com a versão dos arquivos Z, a frase 7 chega à resposta da tese, "YES, THE RED CUBE." (precedida de uma frase "BY ..., I ASSUME YOU MEAN ..." que a tese não tem; por isso ela está no grupo das 5 com texto a mais).
- "build one." constrói a torre (frase 42), e o trecho do "superblock" (43 a 45) sai palavra por palavra como na tese. O trecho do "nice" e do "like" (13 a 16) funciona, e as perguntas sobre os próprios atos do robô (28 a 33) saem palavra por palavra como na tese. Na 23 ("what color is it?"), o programa escolhe o "it" sozinho, avisa a escolha e responde RED, como na tese. Na 35, olha o passado e acha os quatro blocos da tese.
- Nas frases que dá para comparar, o moby dá a resposta da tese em várias que a versão reescrita (1973–74), restaurada em 2024, não dá: 9, 17, 22 e 27 (idênticas) e 7 (com texto a mais). → `05_transcricao_ITS_2024/`

→ `10_moby_rodando/` (`COMO_RODAR.md` passo a passo, `RESULTADOS.md` frase a frase, `tabela_rodada16.tsv`, `transcricoes/rodada16_definitiva.txt`; em inglês: `HOW_TO_RUN.md` e `RESULTS_EN.md`)

## Quem já sabe disso

- Lars Brinkhoff importou o diretório para o its-vault em 2018 (commit `5dc6815`). A issue #147 do PDP-10/its (dez/2018) trata dessa fita e do que havia nela para testar, inclusive o moby dump; o índice do diretório já dizia que a imagem executa o diálogo. Quem lesse os dois tinha a pista.
- Em 2024, o que Eric Swenson recuperou das fitas ToTS e chamou de "the original SHRDLU" (grupo pidp-10, 05/08/2024) é a versão reescrita de 1973–74 (pasta `01`). No mesmo fio, Swenson relata a conversa com Terry Winograd: o roteiro da demonstração foi criado a partir de "interactions with SHRDLU prior to a complete rewrite by Stu Card in 1971 or 1972". A versão anterior a essa reescrita é a que está nos arquivos Z e no moby dump.
- Em dezembro de 2025, Christopher Hesse escreveu que "the available SHRDLU source only partially reproduces the demo" e que "if there was a single version of the source code that reproduces the demo, it seems hard to find today" (affinelayer.com, 11/12/2025). Lars Brinkhoff, no grupo pidp-10 (11/12/2025): "I haven't examined dumped LISP images much, but if I recall they are hard to understand."
- Até 13/10/2026, procurando nas issues do PDP-10/its e do its-vault, no grupo pidp-10, na página de Hesse e na web, não localizei registro público de alguém que tenha identificado os arquivos Z como a versão da tese, decodificado o moby dump ou rodado a imagem. É uma busca, não uma prova de ausência; quem pode confirmar são as pessoas que trabalham nessas fitas. Citações conferidas na fonte: `06_verificacoes/citacoes_2024_2025.txt`.

## Pistas do que ainda falta

- **SMPLACE**: chamada no código da época, não definida em nenhum arquivo nem na imagem.
- **FINDQ**: também não existe em lugar nenhum. Foi reconstruída uma versão de duas linhas, a partir do que o programa passa para ela; com isso a frase 5 acerta. O original continua perdido.
- **Erros no próprio código da época**: a SMONE, a SMPROP, a SMADJ e a SMIT3 tinham erros que derrubam as frases 13, 14, 23 e 42 a 45 (corrigidos, ver `10_moby_rodando/RESULTADOS.md`). Na 23 também foi preciso mudar a WHO: do jeito que está, o teste que a tese descreve nunca dá certo em pergunta de cor. Na 35, uma linha repetida em `z.dictio` apagava a lista de teoremas de #LOC com tempo. Na 37 faltavam três peças que as versões seguintes acrescentaram: o "but" na SMRSQ, um objetivo para "anything" e a mão do robô na SIZE. Isso sugere que a imagem e os arquivos Z são um retrato de pouco antes da versão que gerou o diálogo impresso.
- **Micro-Planner da época, em fonte**: está compilado dentro do moby dump. Os fontes de 1971 (plnr.169–171, set./out. 1971; plnr.176, dez. 1971) estão em https://github.com/MITDDC/plnr-1971-1982 (fitas do MIT, coleção MC-0741); o Planner da imagem é dessa família. O manual de Stanford (SAILDART, `PLNR.RPO[UP,DOC]`, out/1972) diz que o fonte, versão 135, ficava em `PLNR.135[P,RPO]`, e a demo BLOCKS de Winograd em `BLOCK.THM`, `BLOCK.LSP`, `DATA.TW` `[H,RPO]`.
- **Listagens em papel**: Dave McDonald disse ter "hard copy of all of the original that was Lisp source and of all our rewrites" (issue #147).
- **Computer History Museum, coleção Herbert Stoyan**: "SHRDLU program listing" (1974, catálogo 102719668) e a fita "SHRDLU GOERZ" (102719725). São de 1974, provavelmente a versão reescrita.
- **Papéis de Winograd em Stanford (SC1165)**: fita VHS "SHRDLU" (caixa 13) e pastas "MIT/AI" (caixa 29). Abertos para pesquisa.
- **Tapes of Tech Square (MIT)**: 1,3 TB de fitas do AI Lab, de 1973 ao início dos anos 90. Acesso restrito, por pedido.

## O que tem neste repositório

| Pasta | O que é |
|---|---|
| `01_codigo_original_fitas_MIT/` | 21 arquivos do diretório SHRDLU no repositório PDP-10/its, importados **sem edição** (commit `ace4248`, jul/2024). É a versão reescrita de 1973–74. |
| `02_codigo_restaurado_2024_Swenson/` | O mesmo código com as correções de Eric Swenson (22/07 a 22/08/2024) para rodar no MacLisp do ITS emulado. |
| `03_demo_TWDEMO/` | `demo.flick`: roteiro da demonstração gravada (diálogo + cada movimento do braço). `ts.twdemo`: binário dos anos 70 que reproduz a demo; só roda no emulador PDP-10. |
| `04_documentacao_ITS/` | Os dois índices do diretório (`#files.info` e `#files.6`; o segundo data os arquivos Z: "circa January 1971"), manual da CMU (`manual.cmu`), `manual.1`, `mannew.2`, `help.doc`, `minih.doc`. Cópia byte a byte de `doc/shrdlu/` do repositório PDP-10/its. |
| `05_transcricao_ITS_2024/` | Sessão real da versão de 1973–74 rodando no ITS emulado (Swenson, 25/07/2024). |
| `06_verificacoes/` | Comparações mecânicas (diff/grep) que sustentam as afirmações deste arquivo. |
| `07_its_vault_SHRDLU_original/` | O diretório SHRDLU antigo do its-vault: `moby.dump`, os 8 arquivos Z, versões anteriores dos arquivos reescritos, `demo.1` (roteiro anterior ao `demo.flick`), `scene`, `scutil`, `global`, `debug`, `break` etc. |
| `08_its_vault_GRAMAR/` | Os diretórios GRAMAR e GRAMMAR do its-vault: gramática (versões 61–64), interpretador PROGRAMMAR (`progmr` 26–27), `ginter`, `loader`, `conver.sion`. |
| `09_moby_dump_extraido/` | O que foi tirado de dentro do moby dump: as 210 funções interpretadas da imagem em texto MacLisp (`definicoes_interpretadas.lisp`, inclusive as que não vêm dos seis arquivos Z comparados), a ATABLE, o banco do Planner, a lista de funções da imagem, a comparação com os arquivos Z (292 definições, inclusive teoremas) e os scripts que fazem tudo isso. |
| `10_moby_rodando/` | O moby dump rodando no emulador: passo a passo (`COMO_RODAR.md`), resultados frase a frase (`RESULTADOS.md`), os ajustes (um arquivo `.lisp` por intervenção), os scripts das rodadas, os rastreios e as transcrições das rodadas 1 a 16. |

## O que ainda está perdido

- **SHRDLU >** — imagem *"of the most current, best debugged SHRDLU"* (da versão reescrita). Não apareceu.
- **Fonte do TWDEMO** — nenhum foi publicado. Swenson disse em 04/08/2024 que tinha fontes da demo, sem ter tentado compilá-los, e em 04/12/2025: "I wish we had source to TWDEMO, but we don't". O `demo.1` do its-vault é um roteiro anterior ao `demo.flick`.
- **O estado exato que imprimiu o diálogo da tese.** A imagem e os arquivos Z parecem ser de pouco antes dele (ver "Pistas do que ainda falta").

## Os números do MacLisp estão em octal

O MacLisp dessa época lê e imprime números em base 8. `(110 100 0)` no arquivo `data` quer dizer 72, 64, 0 em decimal. Com a conta em octal, os 8 pontos de pegada do `demo.flick` batem com o arquivo `data`. → `06_verificacoes/demo_flick_vs_arquivo_data.txt`

Isso importa para qualquer reconstrução: um port que lê esses números em decimal muda as proporções do mundo.

## Problemas encontrados nas cópias em circulação

1. **`smspec` da distribuição de Stanford (`code.tar`, página de Winograd) está corrompido.** São 25 linhas com bits trocados nas funções SMONE3 ("one"), SMPOSS/SMPOSS2 (possessivo "pyramid's") e SMRELATE (orações relativas). A cópia das fitas do MIT está limpa. O pacote de trabalho deste projeto (`original_maclisp/`, cópia do `code.tar`) e as duas traduções feitas a partir dele herdaram o defeito; qualquer port feito a partir do `code.tar` deve ser conferido. → `06_verificacoes/smspec_corrupcao_stanford.diff`
2. **O port para Common Lisp distribuído na página de Winograd** ("produced by the UMR student project to resurrect SHRDLU", arquivos fornecidos por Greg Sharp) **tem 2 linhas corrompidas** em SMPOSS2 (`SMRUB` e `RETQ AMOB1`). → `06_verificacoes/oraculo_cl_smposs2.diff`
3. Os outros 17 arquivos em comum entre o `code.tar` e as fitas do MIT têm o mesmo código. → `06_verificacoes/pacote_trabalho_vs_fitas_MIT.txt`
4. **O etaoin-shrdlu** (interpretador MacLisp de Peter Cerman, que baixa o `code.tar` e aplica remendos) **corrige 21 das 25 linhas, mas deixa as mesmas 2 linhas de código corrompidas em SMPOSS2** (`SMRUB` e `RETQ AMOB1`; `RETQ` não existe). → `06_verificacoes/etaoin_shrdlu_smspec.txt`

## Lacunas da versão reescrita (1973–74)

Valem para o código das pastas 1 e 2 (e para as cópias de Stanford), não para os arquivos Z:

- 6 funções existem só como aviso "NOT WRITTEN YET": SMTIME, SMTIME2, SMADJGQSHORT, SMADV, SMPLACE, SMTOADJ.
- PLNR-ORIFY: "sorry, plnr-orify not written".
- LASTEVENT e LASTANSEVENT: documentados em DOBACKREF, nunca preenchidos.
- Comentários datados de 1973 e 1974.

→ `06_verificacoes/lacunas_no_codigo.txt`

## Como conferir você mesmo

1. Diretório original (moby dump e arquivos Z): https://github.com/PDP-10/its-vault/tree/master/files/shrdlu. Clique no arquivo e use **Download raw file**.
2. Gramática antiga: https://github.com/PDP-10/its-vault/tree/master/files/grammar.
3. Versão reescrita: https://github.com/PDP-10/its/tree/master/src/shrdlu (sem as edições de 2024: troque `master` por `ace4248fe873b9b86925462f459373cc35a66f41`).
4. Documentação: https://github.com/PDP-10/its/tree/master/doc/shrdlu.
5. Para decodificar o moby dump: compile o `dis10` de https://github.com/larsbrinkhoff/pdp10-its-disassembler (`make dis10`) e rode `./dis10 -Wits moby.dump > moby_dis.txt`. Depois use os scripts de `09_moby_dump_extraido/scripts/` (`mem.py` primeiro).
6. Para rodar a imagem: `10_moby_rodando/COMO_RODAR.md`.

## Origem dos arquivos

- **Redistribuídos sem alteração**, vindos dos repositórios públicos do projeto PDP-10 (material do MIT AI Lab dos anos 1970): as pastas `01` a `05`, `07` e `08` (arquivos das fitas, `moby.dump`, `ts.twdemo`, manuais). Os repositórios de origem estão em "Fontes".
- **Trabalho novo deste repositório** (2026): os scripts, as análises, as comparações (`06`, `09`) e os ajustes e transcrições de `10_moby_rodando/`. Cada ajuste é um arquivo separado; nenhum arquivo original foi editado.

## Fontes

- its-vault, diretório SHRDLU: https://github.com/PDP-10/its-vault/tree/master/files/shrdlu · commit da importação: https://github.com/PDP-10/its-vault/commit/5dc6815
- Discussão no projeto ITS que aponta essa cópia: https://github.com/PDP-10/its/issues/147
- Ferramentas de Lars Brinkhoff para arquivos do PDP-10: https://github.com/larsbrinkhoff/pdp10-its-disassembler
- Código e demo no ITS: https://github.com/PDP-10/its/tree/master/src/shrdlu · commit do TWDEMO: https://github.com/PDP-10/its/commit/4ec85d9e9fda18262c1b8511b7f61d0261dffa64 · PR da restauração: https://github.com/PDP-10/its/pull/2318
- Fontes do Micro-Planner (1971–1982), fitas do MIT: https://github.com/MITDDC/plnr-1971-1982
- Discussões no grupo PiDP-10 (Swenson, Brinkhoff, relato de Winograd): https://groups.google.com/g/pidp-10/c/34dln67jdS4/m/TF-0YUFiBgAJ e https://groups.google.com/g/pidp-10/c/wuuzkAa77Lc=/m/9dBx-mcZAQAJ
- Página de Winograd: https://hci.stanford.edu/winograd/shrdlu/
- Repositório de IA da CMU: https://www.cs.cmu.edu/Groups/AI/areas/classics/shrdlu/0.html
- etaoin-shrdlu (Peter Cerman): https://github.com/pcerman/etaoin-shrdlu
- Affine Layer (Christopher Hesse, dez/2025): https://affinelayer.com/shrdlu/
- Lista histórica de restaurações (ex-Semaphore): https://sites.google.com/site/masteraddressfile/misc/shrdlu
- Fita "SHRDLU GOERZ" no Computer History Museum: https://www.computerhistory.org/collections/catalog/102719725
- Entrevista de Winograd (1991): https://conservancy.umn.edu/handle/11299/107717
- Tese AI-TR-235: https://dspace.mit.edu/handle/1721.1/7095
- Filmes do MIT AI Lab: https://projects.csail.mit.edu/video/history/aifilms/
- Imagem pronta do ITS para o emulador KA10: http://hactrn.kostersitz.com/images/pdp10-ka.tgz
- Manual do Micro-Planner em Stanford (1972), com a localização dos fontes: [saildart.org/PLNR.RPO\[UP,DOC\]1](https://saildart.org/PLNR.RPO[UP,DOC]1)
- "SHRDLU program listing" (1974), coleção Stoyan: https://www.computerhistory.org/collections/catalog/102719668
- Papéis de Terry Winograd em Stanford (SC1165): https://oac.cdlib.org/findaid/ark:/13030/c89k4cp6
- Tapes of Tech Square (MIT): https://archivesspace.mit.edu/repositories/2/resources/1265

## Histórico

- 10–11/10/2026: identificação dos arquivos Z e do moby dump no its-vault; decodificação da imagem; primeira execução no emulador.
- 12/10/2026: rodada 15 (as 46 frases).
- 13/10/2026: rodada 16, com a ANS2 corrigida; contagem estrita (36 / 5 / 5) e inventário completo das intervenções. Publicação deste repositório. Rodadas cumulativas A, B e C (20 / 21 / 27 idênticas). Revisão do texto no mesmo dia: contagem do item 6, números da comparação com a imagem (292/238/234), frase 7, SMPLACE/FINDQ, pastas renumeradas; o item 3 do inventário (READCH) estava descrito como duas intervenções e é uma, em duas linhas: o total passa de 18 para 17.
