(COMMENT A FUNCAO CONJ DE Z GRAMAR COM UM PARENTESE MUDADO DE LUGAR: (SETQ N (CDR N)) FICA DENTRO DA MESMA CLAUSULA QUE (SETQ PREV ...) COMO NO CODIGO COMPILADO DA IMAGEM MOBY DUMP NOS ENDERECOS OCTAIS 263036 A 263046 E NAO COMO CLAUSULA SEPARADA DO COND)
(DEFPROP CONJ
 (LAMBDA (ACONJ)
  (PROG (NBB PC
	     NB
	     NBBB
	     CUT
	     X
	     END
	     FE
	     PREV
	     RES SM)
	(SETQ END (CADR ACONJ))
	(SETQ NBBB (CADAR RE))
	(SETQ NBB N)
	(SETQ RES RE)
   UP	(SETQ PREV (CADDAR (SETQ NB N)))
	(COND
	 ((OR (EQ N END)
	      (EQ (SETQ N (CDR N)) END))
	  (GO LOSE))
	 ((NQ SPEC)
	  (COND
	   ((AND
	     (EQ PREV (QUOTE /,))
	     (OR
	      (CDR RE)
	      (GREATERP
	       (DIFFERENCE (LENGTH NBBB)
			   (LENGTH NBB))
	       4))
	     (MEMQ (CADDAR N)
		   (QUOTE (OR AND NOR BUT)))
	     (NOT (EQ (CDR N) END)))
	    (SETQ PREV
		  (LIST PREV (CADDAR N)))
	    (SETQ N (CDR N)))
	   (T (GO LOSE)))))
	(AND (ATOM PREV)
	     (** N NW (EQ (CADDAR PTW) PREV))
	     (CUT PTW))
	(AND (OR (EQ PREV (QUOTE BUT))
		 (EQ (CADR PREV) (QUOTE BUT)))
	     (NEXTW NOT)
	     (OR (NOT (EQ (SETQ N (CDR N))
			  END))
		 (GO LOSE2))
	     (FQ NEGBUT))
	(SETQ PC RE)
	(OR
	 (SETQ
	  RES
	  (COND
	   ((AND (MEMQ (CAAR ACONJ) (QUOTE (ADJ NUM
				       NOUN
				       PREP
				       VB
				       ADV)))
		 (MEMQ (CAAR ACONJ) (CAAR N)))
	    (PARSE3
	     (APPEND (CAR ACONJ)
		     (QUOTE ((COMPONENT))))
	     T))
	   ((MEMQ (CAAR ACONJ)
		  (QUOTE (NG PREPG ADJG)))
	    (PARSE2 (APPEND (CAR ACONJ)
			    (QUOTE (COMPONENT)))
		    T))
	   ((EQ (CAAR ACONJ) (QUOTE CLAUSE))
	    ((LAMBDA(LASTSENT)(PARSE2
	     (APPEND
	      (CAR ACONJ)
	      (MEET (CAAR RE)
		    (QUOTE (DECLAR IMPER)))
	      (QUOTE (COMPONENT)))
	     T))
(COND((MEMQ 'MAJOR(FE RE))RE)
(LASTSENT)))
)))
	 (GO LOSE2))
	(SETQ NN (NOT (EQ N (SETQ CUT END))))
	(SETQ RE (CONS (CAR RES) RE))
	(COND
	 ((NOT (ATOM PREV))
	  (RETURN
	   (AND
	    (OR SMN (SMCONJ) (ERT SMCONJ))
	    (SETQ
	     RE
	     (FIXX
	      (CONS
	       (CADR PREV)
	       (COND
		((CDDR RE)
		 (QUOTE (COMPOUND LIST)))
		(T (QUOTE (COMPOUND))))))))))
	 ((EQ PREV (QUOTE /,))
	  (COND ((AND NN
		      (EQ (CADDAR N) (QUOTE /,)))
		 (FQ LIST)
		 (GO UP))
		(T (GO LIST))))
	 ((MEMQ PREV (QUOTE (AND OR NOR BUT)))
	  (COND ((EQ BOTH NB) (FQ BOTH)))
	  (COND
	   ((AND
	     NN
	     (OR
	      (EQ (CADDAR N) (QUOTE BUT))
	      (AND
	       (EQ (CADDAR N) PREV)
	       (NOT (AND (EQ BOTH NB)
			 (EQ PREV
			     (QUOTE AND)))))))
	    (FQ LISTA)
	    (F PREV)
	    (GO UP))
	   (T (GO LISTA)))))
   LOSE2(SETQ N NB)
	(COND
	 ((NULL (CDR RE))
	  (COND
	   ((AND (MEMQ (QUOTE INIT) (CAAR RE))
		 (EQ PREV (QUOTE /,)))
	    (SETQ
	     NN
	     (NOT (EQ END
		      (SETQ N (CDR N)))))))
	  (RETURN NIL))
	 ((CQ LISTA) (GO LISTA))
	 ((CQ LIST) (GO LIST)))
   LOSE	(COND ((CDR RE)
	       (SETQ RE (LASTC RE))))
	(SETQ N NBB)
	(RETURN NIL)
   LISTA(RETURN
	 (AND
	  (OR SMN (SMCONJ) (ERT SMCONJ))
	  (SETQ
	   RE
	   (FIXX
	    (APPEND (COND ((CDDR RE)
			   (QUOTE (LIST))))
		    (COND (SPEC (LIST SPEC)))
		    (LIST PREV)
		    (QUOTE (COMPOUND)))))))
   LIST	(COND
	 ((AND (EQ PREV (QUOTE /,))
	       (CDR RE)
	       (NULL (CDDR RE))
	       (EQ (CAAAR RE) (QUOTE NG))
(NOT(OR(ISQ RE PRONG)(ISQ(CDR RE)PRONG)))
(OR	   (AND NN (EQ (CADDAR N) (QUOTE /,))
(OR	    (SETQ
	     NN
	     (NOT (EQ END
		      (SETQ N (CDR N)))))T))
(NULL N)))
	  (RETURN
	   (AND
	    (OR SMN (SMCONJ) (ERT SMCONJ))
	    (SETQ
	     RE
	     (FIXX
	      (QUOTE (APPOSITIVE COMPOUND)))))))
	 ((MEMQ (QUOTE INIT) (CAAR RE))
	  (SETQ NN
		(NOT (EQ END
			 (SETQ N
			       (CDR NBB)))))
	  (COND ((CDR RE)
		 (SETQ RE (LASTC RE))))
	  (RETURN NIL)))
	(GO LOSE)))
 FEXPR)
