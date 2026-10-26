(COMMENT A FUNCAO SHRDLU DE Z SYSCOM COM UMA UNICA DIFERENCA: O TESTE (AND SMN NOFLAG) ANTES DE (ANSWER NOFLAG) COMO NO CODIGO COMPILADO DA IMAGEM MOBY DUMP NOS ENDERECOS OCTAIS 251656 A 251662 QUE TESTAM SMN E DEPOIS NOFLAG ANTES DE CHAMAR ANSWER)
(DEFPROP SHRDLU
	 (LAMBDA NIL
		 (PROG (END SPEC
			    AMB
			    PC
			    TIMAMB
			    BOTH
			    CUT
			    WHO
			    PT
			    PMARK
			    PTW
			    PTR
			    SENT
			    PUNCT
			    H
			    N
			    NB
			    FE
			    SM
			    PD
			    RE
			    MES
			    MESP
			    UNMKD
			    C
			    CUT
			    CURTIME
			    STATE)
		       (OR SMN (AND DISCOURSE (FORGET)))
		  EX   (SETQ LASTSENT C)
		       (OR SMN (CLEANX (Q AMB)
				       (GET (Q AMB) (Q OLD))
				       (PUTPROP (Q AMB)
						(GET (Q AMB) (Q NEW))
						(Q OLD))))
		       (OR SMN
			   (PUTPROP (Q AMB) (GET (Q AMB) (Q MAKESYM)) (Q NEW)))
		  EX2(SETQ BACKREF NIL)
  (SETQ N (SETQ SENT (ETAOIN)))
		       (PRINT (Q *))
		       (AND WALLP (IOC WR) (%) (IOC VT))
		       (SETQ END (SETQ NOFLAG NIL))
		  FOO  (SETQ PD NIL)
		       (COND ((NULL (SETQ C (ERRSET (PARSE CLAUSE MAJOR)))))
			     ((SETQ PT (SETQ C (CAR C)))
			      (SETQ FE (FE C))
			      (SETQ NB SENT)
			      (SETQ H (H C))
			      (SETQ MES (Q NOPE))
			      (COND ((AND SMN NOFLAG) (ANSWER NOFLAG) (SETQ SMN NIL)))
			      (AND SMN (ERTSTOP OK))
			      (AND (EQ (Q ALL) WALLP) (IOC WR) (WALLP C))
			      (IOC VT)
			      (GO EX))
			     (NOFLAG (SETQ SMN (SETQ N SENT)) (GO FOO)))
(COND(NOFLAG(PRINT NOFLAG))
(			     (SAY /
*I DON/'T UNDERSTAND)))
(OR NOSTOP(ERT))(GO EX2)))
	 EXPR)
