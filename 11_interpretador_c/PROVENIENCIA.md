# Proveniência dos arquivos / File provenance

Cada arquivo da pasta, o que ele é e de onde veio. Quando a origem diz "cópia" ou "idêntico", o SHA-256 foi conferido contra o original na montagem desta pasta. O SHA-256 completo de todos os arquivos está em `SHA256SUMS`.

Every file in this folder, what it is and where it came from. "cópia"/"idêntico" (copy/identical) means the SHA-256 was checked against the original when this folder was assembled. Full SHA-256 of every file: `SHA256SUMS`.

| Arquivo | Natureza | Origem / o que é | SHA-256 (início) |
|---|---|---|---|
| `maclisp.c` | interpretador (2026) | o `maclisp_f27.c` da ELCIA, sem mudança | `234cd57c35ae6ce5` |
| `historicos/z.syscom` | histórico | cópia de `07_its_vault_SHRDLU_original/z.syscom` (PDP-10/its-vault) | `3e8811d383cd8de5` |
| `historicos/z.gramar` | histórico | cópia de `07_its_vault_SHRDLU_original/z.gramar` (PDP-10/its-vault) | `d43f0279430fa455` |
| `historicos/z.dictio` | histórico | cópia de `07_its_vault_SHRDLU_original/z.dictio` (PDP-10/its-vault) | `dcded9d5bb56243b` |
| `historicos/z.semant` | histórico | cópia de `07_its_vault_SHRDLU_original/z.semant` (PDP-10/its-vault) | `81c0ca26d4da8ca1` |
| `historicos/z.answer` | histórico | cópia de `07_its_vault_SHRDLU_original/z.answer` (PDP-10/its-vault) | `e47aa3602b0491e8` |
| `historicos/z.blocks` | histórico | cópia de `07_its_vault_SHRDLU_original/z.blocks` (PDP-10/its-vault) | `05db389917bf03f8` |
| `historicos/z.break` | histórico | cópia de `07_its_vault_SHRDLU_original/z.break` (PDP-10/its-vault) | `372b329629a4582f` |
| `historicos/z.mover` | histórico (não carregado: código de máquina do PDP-10) | cópia de `07_its_vault_SHRDLU_original/z.mover` (PDP-10/its-vault) | `13f0ecf19f0e63ce` |
| `historicos/data.5` | histórico | cópia de `07_its_vault_SHRDLU_original/data.5` (PDP-10/its-vault) | `a222112239b7b81a` |
| `historicos/plnr.176` | histórico | cópia de MITDDC/plnr-1971-1982, `plnr/3100015/plnr/plnr.176` (data na fita: 28/12/1971) | `fd96eefebff9586e` |
| `historicos/LICENSE-plnr-MIT.txt` | licença | cópia de MITDDC/plnr-1971-1982, `LICENSE` | `a266f6b177b32c49` |
| `apoio/boot1.lisp` | apoio (2026) | início do Micro-Planner e do Lisp (THINIT, macros de leitura, variáveis) | `cfa55ca2967931e7` |
| `apoio/macr_z.lisp` | apoio (2026) | `macr.1` do PROGRAMMAR adaptado ao formato da gramática Z; TIME como na gramática compilada da imagem | `9b2c85f4bd4a499d` |
| `apoio/reconstruidas.lisp` | apoio (2026) | funções que só existem compiladas na imagem, sem fonte (FE, NB, N, DPRINT2...) | `13b6e2570fe5e05c` |
| `apoio/compiladas.lisp` | apoio (2026) | cópia SUBR das funções que a imagem tinha compiladas | `c49c7847a5502cf5` |
| `apoio/ambiente.lisp` | apoio (2026) | LISTEN e SLEEP (terminal) | `86f135c13f714956` |
| `apoio/variaveis.lisp` | apoio (2026) | as 4 variáveis da rodada 16 (NOSTOP, NOMEM, VALUE, PLANNERSEE) | `88eaf2770007a17c` |
| `apoio/lote.lisp` | apoio (2026) | THERT em lote: a resposta NIL do operador embutida, como a THERT interpretada da imagem | `4c1a08f8bc772dae` |
| `ajustes/rodada16/findq_reconstruida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/findq_reconstruida.lisp` | `1c9d0ae04a89fa88` |
| `ajustes/rodada16/smone_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/smone_corrigida.lisp` | `94ef84deee431f6d` |
| `ajustes/rodada16/smprop_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/smprop_corrigida.lisp` | `16bd7823de5c521b` |
| `ajustes/rodada16/smadj_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/smadj_corrigida.lisp` | `a785dcb50989190c` |
| `ajustes/rodada16/modal_zerado.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/modal_zerado.lisp` | `2d474ccfb73d2658` |
| `ajustes/rodada16/goalize2_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/goalize2_corrigida.lisp` | `522ac84bc54da94c` |
| `ajustes/rodada16/smit3_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/smit3_corrigida.lisp` | `db763f0a7dad1a04` |
| `ajustes/rodada16/who_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/who_corrigida.lisp` | `fe691a1fee7cd41c` |
| `ajustes/rodada16/loc_thmlist_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/loc_thmlist_corrigida.lisp` | `4983546c5f20804c` |
| `ajustes/rodada16/smrsq_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/smrsq_corrigida.lisp` | `63675dc3b27b85c3` |
| `ajustes/rodada16/smng1_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/smng1_corrigida.lisp` | `fef14f978204ce7f` |
| `ajustes/rodada16/size_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/size_corrigida.lisp` | `b0dba12d90c9da13` |
| `ajustes/rodada16/ans2_corrigida.lisp` | ajuste da rodada 16 | idêntico a `10_moby_rodando/ans2_corrigida.lisp` | `0a7ee7c768f38869` |
| `ajustes/rodada16/remendo_ng_no_texto.lisp` | ajuste da rodada 16 | o remendo de `10_moby_rodando/remendo_ng.lisp` (duas palavras de código de máquina) feito nas mesmas duas expressões do texto da gramática | `d6135a58e69ca728` |
| `ajustes/imagem/shrdlu_como_na_imagem.lisp` | como na imagem | SHRDLU com (AND SMN NOFLAG), endereços 251656–251662 | `6bef99cc2b8c319c` |
| `ajustes/imagem/conj_como_na_imagem.lisp` | como na imagem | CONJ com um parêntese no lugar da imagem, endereços 263036–263046 | `e27d817d630617cd` |
| `ajustes/imagem/ng_como_na_imagem.lisp` | como na imagem | NG com (CQ DEF) no lugar de (CQ DET), endereços 260073–260102 | `8877d51eb853681e` |
| `ajustes/imagem/threturn_como_na_imagem.lisp` | como na imagem | THRETURN com o argumento fora do alcance dinâmico, como a FSUBR em 244124 | `11c2e12309c96a4e` |
| `ajustes/imagem/planner_como_na_imagem.lisp` | como na imagem | THVAL (245301), THFAIL (241117) e THGOALF (241757) da imagem; o arquivo `f31_planner_como_na_imagem.lisp` | `8332b32b6d0f9296` |
| `ajustes/imagem/etaoin_como_na_imagem.lisp` | como na imagem | leitor de frases da imagem (ETAOIN 246255, ETCUT, COMPLAIN, ETNEW, ETDO, ADDCHAR); o arquivo `f31_etaoin_como_na_imagem.lisp` | `1c998a967ddb0360` |
| `testes/tabela_rodada16.tsv` | referência | idêntico a `10_moby_rodando/tabela_rodada16.tsv` (tese e rodada 16) | `8ab52c123584df27` |
| `testes/dialogo_1971_entrada.txt` | referência | idêntico a `10_moby_rodando/dialogo_1971_entrada.txt` | `73c06ee138739cd9` |
| `testes/frases_extras.txt` | teste ou documentação (2026) | as 10 frases fora do diálogo (as mesmas digitadas no ITS) | `507409cf091531e4` |
| `testes/teste_minimo_thfail.lisp` | teste ou documentação (2026) | teste mínimo do THFAIL (o mesmo arquivo rodado no ITS) | `109e5c97e3c03e1f` |
| `testes/sonda_turno16.lisp` | teste ou documentação (2026) | sonda da frase 16 (a do ITS, com o THFAIL como FEXPR) | `b2e9b395e4aaf859` |
| `testes/dialogo.py` | teste ou documentação (2026) | digita as frases no interpretador | `e59c247f23a9e4c5` |
| `testes/verificar.py` | teste ou documentação (2026) | compara com a tese e com a rodada 16 | `71ecb13ab1e82d84` |
| `testes/comparar_its.py` | teste ou documentação (2026) | compara com os logs do ITS | `0fe1dcda7f74ae70` |
| `carga.txt` | teste ou documentação (2026) | ordem de carga | `9081f922ee1b77b5` |
| `shrdlu.sh` | teste ou documentação (2026) | modo interativo | `b3ee0835378c4328` |
| `testar.py` | teste ou documentação (2026) | teste completo | `41cc93ec256db814` |
| `README.md` | teste ou documentação (2026) | documentação | `5879170d1ed38905` |
| `NOTICE.md` | teste ou documentação (2026) | licenças | `9157c13bbb2b4dfe` |
| `.gitignore` | teste ou documentação (2026) | ignora build/ e os logs de resultados/ | `21c7bb0d38ffcdc2` |
| `evidencias/its/COMO_REPETIR_NO_ITS.md` | evidência: ITS (2026) | log bruto ou instruções | `aed76d4e90b7ba51` |
| `evidencias/its/dialogo_ate16.log` | evidência: ITS (2026) | log bruto ou instruções | `c150e9a7f534bde9` |
| `evidencias/its/dialogo_extras.log` | evidência: ITS (2026) | log bruto ou instruções | `aa223743dacd2a1e` |
| `evidencias/its/roteiros/boot_moby_f31.sh` | roteiro do ITS (2026) | como rodou | `77bf122849f1a84e` |
| `evidencias/its/roteiros/dialogo_extra.py` | roteiro do ITS (2026) | como rodou | `d86bede88f77aed2` |
| `evidencias/its/roteiros/frases_extras.txt` | roteiro do ITS (2026) | como rodou | `507409cf091531e4` |
| `evidencias/its/roteiros/sessao_extras.sh` | roteiro do ITS (2026) | como rodou | `2f3c923a2cbba897` |
| `evidencias/its/roteiros/sessao_turno16.sh` | roteiro do ITS (2026) | como rodou | `8215bb334cf4d0c0` |
| `evidencias/its/roteiros/sonda_turno16_ITS.lisp` | roteiro do ITS (2026) | como rodou | `48e299c4adbbb309` |
| `evidencias/its/roteiros/teste_minimo.sh` | roteiro do ITS (2026) | como rodou | `58744be57246b4e5` |
| `evidencias/its/roteiros/teste_minimo_thfail.lisp` | roteiro do ITS (2026) | como rodou | `109e5c97e3c03e1f` |
| `evidencias/its/sessao_extras_ITS.txt` | evidência: ITS (2026) | log bruto ou instruções | `db57603ee0b8e145` |
| `evidencias/its/sessao_turno16_ITS.txt` | evidência: ITS (2026) | log bruto ou instruções | `9ca9d9baf5d44e5c` |
| `evidencias/its/teste_minimo_ITS.txt` | evidência: ITS (2026) | log bruto ou instruções | `b72871867e377558` |
| `evidencias/moby/desmonta.py` | evidência: imagem MOBY | listagem ou ferramenta de desmontagem | `faa07d9d8df0ab2d` |
| `evidencias/moby/enderecos_de_entrada.txt` | evidência: imagem MOBY | listagem ou ferramenta de desmontagem | `cc9c654f93dbafc7` |
| `evidencias/moby/listagens_moby.txt` | evidência: imagem MOBY | listagem ou ferramenta de desmontagem | `0b59e47da1df0fc3` |
| `evidencias/moby/verificar_listagens.py` | evidência: imagem MOBY | listagem ou ferramenta de desmontagem | `42032db3f3e84edb` |
| `evidencias/segunda_sessao_ITS_chatgpt/ITS_TURNO16_SEM_INVERSAO.txt` | evidência: ITS, ChatGPT (2026) | como veio no pacote dele | `5a75083068296863` |
| `evidencias/segunda_sessao_ITS_chatgpt/LEIA.md` | evidência: ITS, ChatGPT (2026) | o que foi conferido | `5c7421cabde04eb5` |
| `evidencias/segunda_sessao_ITS_chatgpt/PROVA_DIRETA_ITS_APOS15.txt` | evidência: ITS, ChatGPT (2026) | como veio no pacote dele | `22d655a2910d589f` |
| `evidencias/segunda_sessao_ITS_chatgpt/TRANSCRICAO_ITS_1_A_15.txt` | evidência: ITS, ChatGPT (2026) | como veio no pacote dele | `bc9eebda2a4e5e07` |
| `evidencias/segunda_sessao_ITS_chatgpt/console_daemon.log` | evidência: ITS, ChatGPT (2026) | como veio no pacote dele | `8b62b52102098726` |

A pasta `resultados/` é refeita a cada `python3 testar.py` e por isso fica fora do `SHA256SUMS`. / `resultados/` is regenerated by `python3 testar.py`, so it is not in `SHA256SUMS`.
