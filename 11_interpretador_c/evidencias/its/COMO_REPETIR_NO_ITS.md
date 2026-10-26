# Como repetir no ITS as três provas desta pasta

As três sessões rodaram em 25/10/2026, no emulador KA10 do projeto PDP-10/its, com a imagem `OSHRDL; MOBY DUMP`
(o `moby.dump` da pasta `07_its_vault_SHRDLU_original/`, SHA-256 `6d962b97b8c5630df7f5a52a3b67b0d96b5664ed0bdfd4f5361c7a43cb0ef0a3`).

| Sessão | Roteiro | Log bruto |
|---|---|---|
| Teste mínimo do THFAIL, num MOBY recém-carregado, sem nenhum ajuste de 2026 | `roteiros/teste_minimo.sh` | `teste_minimo_ITS.txt` |
| Frases 1 a 16 com os ajustes da rodada 16 e a sonda da frase 16 | `roteiros/sessao_turno16.sh` | `sessao_turno16_ITS.txt` (e `dialogo_ate16.log`) |
| 10 frases fora do diálogo da tese, com os ajustes da rodada 16 | `roteiros/sessao_extras.sh` | `sessao_extras_ITS.txt` (e `dialogo_extras.log`) |

## 1. Montar o ITS

Siga `../../10_moby_rodando/COMO_RODAR.md` (em inglês, `HOW_TO_RUN.md`), passos 1 a 5.
No fim, `:listf oshrdl;` deve mostrar `MOBY DUMP`. Emulador e ITS: https://github.com/PDP-10/its

## 2. Montar a pasta de trabalho

Chame de `T` a pasta de trabalho (aqui era `/home/claude/work/its_tape`).

1. Copie para `T` os arquivos da pasta `10_moby_rodando/`: `its.sh`, `dialogo2.py`, `run.ini`, `smngof_z.lisp`,
   `fixes_ambiente.lisp`, `remendo_ng.lisp` e os 13 ajustes (`findq_reconstruida.lisp` a `ans2_corrigida.lisp`).
2. Crie `T/f31/` e copie para lá os arquivos de `roteiros/`.
   Renomeie `sonda_turno16_ITS.lisp` para `sonda_turno16.lisp` (é o nome que o `sessao_turno16.sh` usa).

## 3. Ajustar os caminhos

Os roteiros guardam os caminhos desta máquina. Troque a linha `T=` de cada `.sh`, os caminhos do `its.sh`
(pasta do ITS e `console.log`), o `LOG` do `dialogo2.py` e do `dialogo_extra.py`, o caminho do
`dialogo_1971_entrada.txt` no `dialogo2.py` e o da fita no `run.ini` (linha `at mta0`).

## 4. Antes de ligar

Confira que nenhum emulador está rodando: `ps -eo pid,args | grep "[p]dp10-ka"`.
Nunca rode dois emuladores com os mesmos discos `rp03.*`: o disco do ITS estraga.

## 5. Rodar e comparar

```sh
bash T/f31/teste_minimo.sh minimo.txt          # uns 3 minutos
bash T/f31/sessao_turno16.sh turno16.txt       # uns 10 minutos
bash T/f31/sessao_extras.sh extras.txt         # uns 10 minutos
```

Depois, na pasta `11_interpretador_c/`, com as saídas do `testar.py` em `resultados/`:

```sh
python3 testes/comparar_its.py minimo minimo.txt  resultados/gcc_teste_minimo.txt
python3 testes/comparar_its.py sonda  turno16.txt resultados/gcc_sonda_frase16.txt
python3 testes/comparar_its.py extras extras.txt  resultados/gcc_frases_extras.txt
```

Esperado: `(#FFF 7)` nos dois lados; a sonda igual sem espaços e linha a linha (20 eventos, 32 linhas);
10 de 10 frases iguais.

## Observações

- O MacLisp do ITS quebra as linhas na coluna 72, até no meio de um átomo. Por isso a comparação da sonda
  junta cada linha de 72 caracteres do ITS com a seguinte.
- A sonda do ITS (`roteiros/sonda_turno16_ITS.lisp`) e a do C (`../../testes/sonda_turno16.lisp`) diferem numa
  linha só: no ITS o THFAIL é FSUBR (compilado), no C é FEXPR.
- A sonda não muda nenhuma resposta: as frases 1 a 16 da sessão com sonda são iguais às da rodada 16.
- O `dialogo_extra.py` é o `dialogo2.py` da pasta 10 lendo as frases de um arquivo dado (variável `FRASES`).
