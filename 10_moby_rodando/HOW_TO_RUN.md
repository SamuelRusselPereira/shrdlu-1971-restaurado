# How to run the moby dump (thesis-era SHRDLU, 1970–71) on a PDP-10 emulator

English version of `COMO_RODAR.md`. Tested 10–13 October 2026 on Linux (Ubuntu) with the KA10 emulator of the PDP-10/its project.

## 1. Tools
- Packages: git, gcc, make, expect, tcl, libsdl2-dev, libsdl2-image-dev, libsdl2-net-dev, tmux.

## 2. Emulator and a ready-made ITS disk image
- `git clone --depth 1 https://github.com/PDP-10/its.git && cd its`
- `git submodule update --init --depth 1 tools/sims tools/itstar`
- `make -C tools/sims pdp10-ka` (the KA10 emulator)
- `curl -O http://hactrn.kostersitz.com/images/pdp10-ka.tgz && tar xzf pdp10-ka.tgz`
- Keep `pdp10-ka.tgz`. If the ITS disk gets corrupted, extract only the `rp03.*` files again.
- `mkdir -p out/pdp10-ka` and create links in `out/pdp10-ka/` to `rp03.0`–`rp03.3` and `dskdmp.rim` from `pdp10-ka/`.

## 3. A tape with the old SHRDLU directory
- `make -C tools/itstar`
- Copy the files of `07_its_vault_SHRDLU_original/` into a folder named `oshrdl/`.
- `tools/itstar/itstar -cf oshrdl.tape -C . oshrdl`
- Set the tape path in `run.ini` (the `at mta0 ...` line).

## 4. Boot ITS
- `tools/sims/BIN/pdp10-ka run.ini`
- When `DSKDMP` appears, wait about 5 seconds before typing. Characters typed too early are lost and DSKDMP answers `FNF`.
- Type `its` and Enter; then `styo+2/popj p,` and Enter; then ESC and `g`.
- Wait for `IN OPERATION` and press Ctrl-Z.
- `:login yourname`

## 5. Copy the tape to the ITS disk (first time only)
- `:dump` → `load links crdir sorry` → at `FILE=` type `*;* *` → wait for `E-O-T` → `quit`
- Check with `:listf oshrdl;` (`MOBY DUMP` must be listed).

## 6. Load and start the moby dump
- `:job moby` → `:load oshrdl;moby dump` → ESC and `g`
- Type, one at a time: `(SETQ NOSTOP T)` `(SETQ NOMEM NIL)` `(SETQ PLANNERSEE NIL)` `(SETQ VALUE NIL)`
- Type the definition of `SMNGOF` from `smngof_z.lisp`. It is Winograd's own version, from the Z files. The one stored in the image hangs Micro-Planner at sentence 7.
- Type the two lines of `fixes_ambiente.lisp`. They make READCH return digits as numbers, as the code expects.
- Type the line of `findq_reconstruida.lisp`. **This is a reconstruction**, not 1971 code (see `RESULTADOS.md`). Without it sentence 5 fails.
- Type the lines of `smone_corrigida.lisp`, `smprop_corrigida.lisp`, `smadj_corrigida.lisp`, `modal_zerado.lisp` and `goalize2_corrigida.lisp`. **These are fixes of ours** (see `RESULTADOS.md`). Without them sentences 13–16 and 42–45 fail.
- Type the two lines of `remendo_ng.lisp`. They change two words of the compiled grammar with `DEPOSIT` (see `RESULTADOS.md`). Without them sentences 28–33 fail.
- Type the lines of `smit3_corrigida.lisp` and `who_corrigida.lisp`. **Changes of ours** (see `RESULTADOS.md`). Without them sentence 23 asks which "it" instead of answering.
- Type the line of `loc_thmlist_corrigida.lisp`. It gives the relation #LOC back the theorem list of line 892 of `z.dictio` (see `RESULTADOS.md`). Without it sentence 35 finds zero blocks.
- Type the lines of `smrsq_corrigida.lisp`, `smng1_corrigida.lisp` and `size_corrigida.lisp`. **Fixes of ours**, made as in the later versions of the program (see `RESULTADOS.md`). Without them sentence 37 gives an error.
- Type the line of `ans2_corrigida.lisp`. One restoration (the #VAGUE mark is found again) and one reconstruction from the thesis, section 4.4.2 (HOW MANY answers "OF THEM"); see `RESULTADOS.md`. Without it sentences 16 and 46 say "OF THEM" where the thesis says "THINGS".
- `(SHRDLU)`
- `READY` appears. Type sentences in CAPITALS, ending with `.` or `?`.

## Warnings
- **Never run two emulators at the same time on the same disk.** Both write to the `rp03.*` files and the ITS disk gets corrupted. It happened here once; the disk was rebuilt from `pdp10-ka.tgz`.
- Closing the tmux window does not close the emulator. It keeps running hidden. Before starting another one, kill the process: `pkill -x pdp10-ka`.

## Notes
- `NOMEM NIL` turns on the event memory (it is off in the image).
- `VALUE NIL` works around a detail of 1971 MacLisp: the second element of an empty list is not empty, it is the symbol VALUE. Without this sentence 3 gives an error.
- Sometimes the program stops after the `*` without answering (it seems to wait for the 340 display, which we do not have). A space unblocks it.
- At `>>> ... LISTENING` (a Micro-Planner break), type `NIL` and a space: the goal fails and the program goes on.
- Ctrl-G returns to Lisp top level. From there a function can be redefined and the dialogue resumed with `(SHRDLU)`. In the test made here, the world and dialogue state were kept.

## Scripts
- `its.sh`: talks to the ITS console inside tmux (`start`, `send`, `cr`, `esc`, `ctrl`, `cap`, `stop`). `stop` and `start` kill the old emulator.
- `restaura_disco.sh`: boots ITS with the clean disk and copies the tape to the disk (step 5).
- `boot_moby.sh`: boots ITS, logs in and loads the image with the dialogue switches.
- `dialogo2.py`: types the thesis dialogue sentence by sentence and records the transcript. Answers "2" to the clarification question, gives the two requested spaces and sends a space when the program stalls.
- `compara.py`: compares a transcript with the thesis, sentence by sentence, by position.
- `rodadas_cumulativas.sh` and `cum_boot.sh`: the three runs of the cumulative table (A: original + environment only; B: + FINDQ; C: + 6 restorations), with `ans2_restauracao.lisp` (only the restoration part of ANS2). Transcripts in `transcricoes/cumulativa_A.txt` etc.
- `rodada16_completa.sh`: the complete, definitive run 16 (boots, loads every fix, types the 46 sentences). `rodada10_completa.sh` to `rodada15_completa.sh` are the earlier ones.
- `ponto12.sh`, `ponto22.sh`, `ponto27.sh`, `ponto34.sh`, `ponto36.sh` and `ponto36b.sh`: run up to sentence 12, 22, 27, 34 or 36 and save the job as a checkpoint (`:pdump oshrdl;s12 dump`). `carrega_ponto.sh s12` reloads it. Each one loads the fixes that existed when it was made: `ponto36b.sh` has all of run 14, `ponto34.sh` those of run 13; the older ones lack the later fixes.
- `trace_p1.lisp`, `trace_ans1.lisp`, `trace_ansthm.lisp`: traces that print the arguments of P1, ANS1 and ANSTHM (load after turning tracing on).
- `shrdlu2.lisp`: returns to the dialogue without clearing the conversation (the function `(SHRDLU)` clears it).
- `trace_defs.lisp` and `trace_lista.lisp`: the tracer. After loading them, `(MAPCAR (FUNCTION TR) TRLIST)` turns it on and `(MAPCAR (FUNCTION UNTR) TRLIST)` off.
- `trace_valores.lisp`: a tracer that also prints the arguments and the value of each call (`TRV`). After loading it, turn it on for the functions of interest, e.g. `(MAPCAR (FUNCTION TRV) (QUOTE (ANS1 ANSX THVAL2)))`.
- `asmuch_teste_37.lisp`: a test of the "as wide as" theorem (TC-ASMUCH). **Do not use in the normal run**: it changes no answer, and the bug it fixes also exists in the rewritten version.
- `dialogo_1971_entrada.txt` (the sentences the script types) and `dialogo_1971.tsv` (the thesis answers, with page numbers).
- The scripts use the paths of this machine (`/home/claude/...`). Adjust the `T=` line of each one and the paths at the top of `dialogo2.py` and `compara.py`.
