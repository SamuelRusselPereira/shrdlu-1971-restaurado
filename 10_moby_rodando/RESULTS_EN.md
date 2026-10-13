# Run 16 (13 October 2026): sentence-by-sentence results, in English

English summary of `RESULTADOS.md` (Portuguese; the full explanation of every intervention is there). Fresh session, image just loaded, the 46 sentences of the thesis dialogue (AI-TR-235, section 1.3, pp. 35–60) typed in order by `dialogo2.py`. Transcript: `transcricoes/rodada16_definitiva.txt`; data: `tabela_rodada16.tsv`.

Numbering is the thesis's own (it has no item 38). Comparison ignores only letter case and punctuation.

**Strict count: 36 identical, 5 with the thesis answer plus an extra sentence, 5 different.**

## What is typed over the untouched original image

Nothing is changed in the original files; everything is redefined at run time by `rodada16_completa.sh`.

1. Four variables: `NOSTOP T`, `NOMEM NIL`, `VALUE NIL`, `PLANNERSEE NIL`.
2. Winograd's own code, taken from the Z files: `SMNGOF` (1). The copy stored in the image is older and hangs Micro-Planner at sentence 7.
3. MacLisp/ITS environment workaround: `READCH` redefined over the original (two lines: the first saves the original READCH as OLDREADCH, the second defines the new one, which turns the digit read into a number) (1).
4. A function that exists nowhere, reconstructed: `FINDQ`, two lines (1). Without it sentence 5 fails.
5. Fixes of errors in the period code (restoration): `SMONE`, `SMPROP`, `SMADJ`, `SMIT3`, the theorem list of `#LOC`, `ANS2` part 1 (6).
6. Justified choices (reconstruction): `WHO`, MODAL reset after each answer, `GOALIZE2`, `SMRSQ`, `SMNG1`, `SIZE`, `ANS2` part 2, and the patch to the compiled grammar (two machine words changed with DEPOSIT, counted as one intervention: one file, `remendo_ng.lisp`) (8).

Total: 17 code interventions plus 4 variables. Each one is explained in `RESULTADOS.md`, with its reason and its effect sentence by sentence; one `.lisp` file per intervention in this folder.

## Cumulative table: what comes from 1971 and what comes from 2026

Three more fresh-session runs (13 October 2026) with growing subsets of the fixes (`rodadas_cumulativas.sh`; in run C, ANS2 enters with its restoration part only, `ans2_restauracao.lisp`):

| Run | Loaded | Identical | Extra sentence | Different |
|---|---|---|---|---|
| A | original + environment only: 4 variables, `READCH`, `SMNGOF` from the Z files | **20** | 4 | 22 |
| B | A + reconstructed `FINDQ` (2 lines) | **21** | 4 | 21 |
| C | B + 6 fixes of period-code errors | **27** | 5 | 14 |
| 16 | C + 8 reconstruction choices | **36** | 5 | 5 |

FINDQ changes only sentence 5. The 6 fixes add 13, 35 and 42–45 (and move 14 from error to "extra sentence"). The 8 choices add 15, 16, 23 and 28–33; of these, 28–33 depend on the compiled-grammar patch and on the SMIT3/WHO changes. The dialogue never stalled in any run: all 46 sentences were typed and answered (with an error or "I DON'T UNDERSTAND" where a fix is missing). Transcripts: `transcricoes/cumulativa_A.txt`, `_B`, `_C`; tables: `tabela_cumulativa_*.tsv`.

## The 46 sentences

| # | Input | Thesis answer | Moby dump, run 16 | Status |
|---|---|---|---|---|
| 1 | pick up a big red block. | OK. | OK. | identical |
| 2 | grasp the pyramid. | I DON'T UNDERSTAND WHICH PYRAMID YOU MEAN. | I DON'T KNOW WHICH PYRAMID YOU MEAN. | different |
| 3 | find a block which is taller than the one you are holding and put it into the box. | BY "IT", I ASSUME YOU MEAN THE BLOCK WHICH IS TALLER THAN THE ONE I AM HOLDING. OK. | OK. BY "IT", I ASSUME YOU MEAN THE BLOCK WHICH IS TALLER THAN THE ONE I AM HOLDING. OK. | thesis answer + extra sentence |
| 4 | what does the box contain? | THE BLUE PYRAMID AND THE BLUE BLOCK. | THE BLUE PYRAMID AND THE BLUE BLOCK. | identical |
| 5 | what is the pyramid supported by? | THE BOX. | THE BOX. | identical |
| 6 | how many blocks are not in the box? | FOUR OF THEM. | FOUR OF THEM. | identical |
| 7 | is at least one of them narrower than the one which i told you to pick up? | YES, THE RED CUBE. | BY "THEM", I ASSUME YOU MEAN THE HOW MANY BLOCKS AM NOT IN THE BOX. YES, THE RED CUBE. | thesis answer + extra sentence |
| 8 | is it supported? | YES, BY THE TABLE. | BY "IT", I ASSUME YOU MEAN THE AT LEAST ONE OF THEM NARROWER THAN THE ONE WHICH YOU TOLD I TO PICK UP. YES, BY THE TABLE. | thesis answer + extra sentence |
| 9 | can the table pick up blocks? | NO. | NO. | identical |
| 10 | can a pyramid be supported by a block? | YES. | YES. | identical |
| 11 | can a pyramid support a pyramid? | I DON'T KNOW. | I DON'T KNOW. | identical |
| 12 | stack up two pyramids. | I CAN'T. | I CAN'T. | identical |
| 13 | the blue pyramid is nice. | I UNDERSTAND. | I UNDERSTAND. | identical |
| 14 | i like blocks which are not red, but i don't like anything which supports a pyramid. | I UNDERSTAND. | I UNDERSTAND. I UNDERSTAND. | thesis answer + extra sentence |
| 15 | do i like the box? | NO. | NO. | identical |
| 16 | do i like anything in the box? | YES, TWO THINGS: THE BLUE BLOCK AND THE BLUE PYRAMID. | YES, TWO THINGS: THE BLUE BLOCK AND THE BLUE PYRAMID | identical |
| 17 | will you please stack up both of the red blocks and either a green cube or a pyramid? | OK. | OK. | identical |
| 18 | which cube is sitting on the table? | THE LARGE GREEN ONE WHICH SUPPORTS THE RED PYRAMID. | THE LARGE GREEN ONE WHICH SUPPORTS THE RED PYRAMID. | identical |
| 19 | is there a large block behind a pyramid? | YES, THREE OF THEM: A LARGE RED ONE, A LARGE GREEN CUBE AND THE BLUE ONE. | YES, TWO OF THEM: A LARGE RED ONE AND THE BLUE ONE. | different |
| 20 | put a small one onto the green cube which supports a pyramid. | OK. | OK. | identical |
| 21 | put the littlest pyramid on top of it. | OK. | OK. | identical |
| 22 | does the shortest thing the tallest pyramid's support supports support anything green? | YES, THE GREEN PYRAMID. | YES, THE GREEN PYRAMID. | identical |
| 23 | what color is it? | BY "IT", I ASSUME YOU MEAN THE SHORTEST THING THE TALLEST PYRAMID'S SUPPORT SUPPORTS. RED | BY "IT", I ASSUME YOU MEAN THE SHORTEST THING THE TALLEST PYRAMID"S SUPPORT SUPPORTS. RED | identical |
| 24 | how many things are on top of green cubes? | I'M NOT SURE WHAT YOU MEAN BY "ON TOP OF" IN THE PHRASE "ON TOP OF GREEN CUBES". DO YOU MEAN: 1 - DIRECTLY ON THE SURFACE 2 - ANWHERE ON TOP OF ? [2] THREE OF THEM. | THREE OF THEM | different |
| 25 | had you touched any pyramid before you put the green one on the little cube? | YES, THE GREEN ONE. | YES, THE GREEN ONE. | identical |
| 26 | when did you pick it up? | WHILE I WAS STACKING UP THE RED CUBE, A LARGE RED BLOCK AND A LARGE GREEN CUBE. | BY "IT", I ASSUME YOU MEAN THE PYRAMID BEFORE I PUT THE GREEN ONE ON THE LITTLE CUBE. WHILE I WAS STACKING UP THE RED CUBE , A LARGE RED BLOCK AND A LARGE GREEN CUBE. | thesis answer + extra sentence |
| 27 | why? | TO GET RID OF IT. | TO GET RID OF IT. | identical |
| 28 | why did you do that? | TO CLEAN OFF THE RED CUBE. | TO CLEAN OFF THE RED CUBE. | identical |
| 29 | why did you clear off that cube? | TO PUT IT ON A LARGE GREEN CUBE. | TO PUT IT ON A LARGE GREEN CUBE. | identical |
| 30 | why did you do that? | TO STACK UP THE RED CUBE , A LARGE RED BLOCK AND A LARGE GREEN CUBE. | TO STACK UP THE RED CUBE , A LARGE RED BLOCK AND A LARGE GREEN CUBE. | identical |
| 31 | why did you do that? | BECAUSE YOU ASKED ME TO. | BECAUSE YOU ASKED ME TO. | identical |
| 32 | how did you do it? | BY PUTTING A LARGE RED BLOCK ON THE TABLE ; THEN LETTING GO OF IT ; THEN PUTTING A LARGE GREEN CUBE ON IT ; THEN LETTING GO OF THAT CUBE ; THEN PUTTING THE RED CUBE ON THAT CUBE ; THEN LETTING GO OF THAT CUBE. | BY PUTTING A LARGE RED BLOCK ON THE TABLE ; THEN LETTING GO OF IT ; THEN PUTTING A LARGE GREEN CUBE ON IT ; THEN LETTING GO OF THAT CUBE ; THEN PUTTING THE RED CUBE ON THAT CUBE ; THEN LETTING GO OF THAT CUBE. | identical |
| 33 | how many objects did you touch while you were doing it? | FOUR OF THEM. | FOUR OF THEM. | identical |
| 34 | what did the red cube support before you started to clean it off? | THE GREEN PYRAMID. | THE GREEN PYRAMID. | identical |
| 35 | there were five blocks to the left of the box then. | NO, ONLY FOUR OF THEM: THE RED CUBE , TWO LARGE GREEN CUBES AND A LARGE RED BLOCK. | NO, ONLY, FOUR OF THEM: THE RED CUBE , TWO LARGE GREEN CUBES AND A LARGE RED BLOCK. | identical |
| 36 | put the blue pyramid on the block in the box. | OK. | OK. | identical |
| 37 | is there anything which is bigger than every pyramid but is not as wide as the thing that supports it? | YES, THE BLUE BLOCK. | YES, FOUR THINGS: THE TABLE , THE BOX , A LARGE RED BLOCK AND THE BLUE BLOCK. | different |
| 39 | does a steeple | SORRY, I DON'T KNOW THE WORD "STEEPLE". PLEASE TYPE TWO SPACES. | SORRY, I DON'T KNOW THE WORD "STEEPLE". PLEASE TYPE TWO SPACES. | identical |
| 40 | a "steeple" is a stack which contains two green cubes and a pyramid. | I UNDERSTAND. | I UNDERSTAND. | identical |
| 41 | are there any steeples now? | NO. | NO. | identical |
| 42 | build one. | OK. | OK. | identical |
| 43 | call the biggest block "superblock". | OK. | OK. | identical |
| 44 | have you picked up superblock since we began? | YES. | YES. | identical |
| 45 | why did you drop it? | BY "IT", I ASSUME YOU MEAN THE SUPERBLOCK. I CAN'T EXPLAIN A NON-EXISTENT EVENT. | BY "IT", I ASSUME YOU MEAN THE SUPERBLOCK. I CAN'T EXPLAIN A NON-EXISTENT EVENT. | identical |
| 46 | is there anything to the right of the red pyramid? | YES, FOUR THINGS: THE BOX , THE RED CUBE, THE BLUE PYRAMID AND THE BLOCK NAMED SUPERBLOCK | YES, FOUR THINGS: THE BOX , THE RED CUBE , THE BLUE PYRAMID AND THE BLUE BLOCK. | different |
| 47 | thank you. | YOU'RE WELCOME! | YOU'RE WELCOME! | identical |

## Notes on the ten that are not identical

- **3, 7, 8, 14, 26 (extra sentence):** 3 has a spare "OK." at the start; 14 prints "I UNDERSTAND." twice; 7, 8 and 26 add a "BY ..., I ASSUME YOU MEAN ..." sentence with broken English ("THE HOW MANY BLOCKS AM NOT IN THE BOX", "WHICH YOU TOLD I TO PICK UP"). These are visible defects of the program as it runs here.
- **2:** one word differs: the program says "KNOW", the printed thesis says "UNDERSTAND". "KNOW" is what the source (`z.answer`, function ANSWER) says.
- **19:** lists two objects instead of three.
- **24:** does not ask the clarification question ("DO YOU MEAN: 1 - DIRECTLY ON THE SURFACE 2 - ANYWHERE ON TOP OF?"); it answers "THREE OF THEM" directly, which is the thesis's final answer. The question is asked by YELL (`z.answer`, line 725) only when the sentence reaches it with two interpretations of similar plausibility; it does not depend on NOSTOP. The cause, upstream in the semantic analysis of "on top of", has not been investigated.
- **37:** lists four objects instead of one; by the program's own data, the other three also satisfy the conditions.
- **46:** calls the blue block "THE BLUE BLOCK" instead of "THE BLOCK NAMED SUPERBLOCK". The database shows `(#CALL :B6 SUPERBLOCK)`: the program named the big red block, not the blue one as in the thesis, because "biggest" uses #SIZE, the sum of the three dimensions (thesis section 3.4.2), and with the data file's sizes B6 wins. No data was changed to force the thesis's result.
