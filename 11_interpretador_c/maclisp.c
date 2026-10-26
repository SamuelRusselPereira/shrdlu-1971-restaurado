/*  maclisp.c — um interpretador de MacLisp de 1971, em C puro, feito para
 *  carregar e executar o código original do SHRDLU (arquivos Z do its-vault,
 *  Micro-Planner plnr.176 e o PROGRAMMAR que está dentro de z.syscom).
 *
 *  Decisões de fidelidade (ver OBSERVACOES.md):
 *   - números lidos e impressos em octal (IBASE = BASE = 8); "10." é decimal;
 *   - '/' escapa o caractere seguinte; ';' e ':' são caracteres comuns até que
 *     um (SSTATUS MACRO ...) diga o contrário, como no Lisp da época;
 *   - o valor de um símbolo é a propriedade VALUE da sua lista de propriedades,
 *     por isso (DEFPROP X 3 VALUE) dá valor a X, como em z.syscom;
 *   - (CDR símbolo) é a lista de propriedades e (CDR NIL) é NIL;
 *   - ligação dinâmica rasa (shallow binding) com pilha de restauração;
 *   - a definição mais nova de uma função vence: PUTPROP põe a propriedade no
 *     início da lista, e a busca da função toma a primeira que achar;
 *   - READCH devolve um número para dígitos, como z.syscom espera (NUMBERP CHAR);
 *   - linhas terminam em CR LF para quem lê caractere a caractere (ITS).
 *  Sem coletor de lixo: a memória é alocada em blocos e nunca devolvida.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <stdint.h>

/* ───────────────────────── objetos ───────────────────────── */
typedef struct Obj Obj;
typedef Obj *L;
enum { T_CONS = 1, T_SYM, T_FIX, T_FLO, T_SUBR };
enum { K_SUBR = 1, K_FSUBR, K_LSUBR };          /* tipos de função em C */

struct Obj {
    int type;
    union {
        struct { L car, cdr; } c;
        struct { char *name; L plist; L valcell; int macro; } s;   /* valcell: a célula cujo CAR é o valor */
        long fix;
        double flo;
        struct { const char *name; int kind, min, max, arity;
                 union { L (*f0)(void); L (*f1)(L); L (*f2)(L,L);
                         L (*f3)(L,L,L); L (*f4)(L,L,L,L); } fn; } subr;
    } u;
};

static L NIL, T_, UNBOUND, DOT_MARK;
static L S_QUOTE, S_LAMBDA, S_EXPR, S_FEXPR, S_SUBR, S_FSUBR, S_LSUBR, S_MACRO, S_VALUE, S_FUNCTION;
static L S_DOT, S_COND, S_PROG, S_GO, S_RETURN, S_SETQ, S_AND, S_OR, S_ERRSET, S_ERR, S_IBASE, S_BASE, S_NOPOINT;
static L S_PNAME, S_READ, S_PRIN1, S_T;

/* ───────────────────────── memória ───────────────────────── */
#define CHUNK (1 << 22)
static char *heap_ptr, *heap_end;
static long n_alloc;
/* F22: diagnostico exclusivamente observacional, em arquivo separado. */
static FILE *who_trace_fp;
/* F23: diagnostico de proveniencia independente do F22, somente opt-in. */
static FILE *f23_db_fp;
/* F24: eventos correlacionados por identidade de objeto, somente opt-in. */
static FILE *f24_fp;
/* F25: diagnostico de despacho e ambiente do Micro-Planner. */
static FILE *f25_fp;
static unsigned long long f25_events;
static unsigned long long f24_events;
static unsigned long long f23_db_events;
static unsigned long long who_trace_events;
static long n_evals;

static void *halloc(size_t n) {
    /* F15: cada arena tem CHUNK bytes. Um pedido maior nao pode ser
       devolvido por malloc(CHUNK), pois permitiria escrita fora da arena.
       Conferir o limite antes do arredondamento evita overflow de size_t. */
    if (n > (size_t)CHUNK) {
        fprintf(stderr, "alocacao maior que arena: %lu bytes (limite %lu)\n",
                (unsigned long)n, (unsigned long)CHUNK);
        exit(2);
    }
    n = (n + 15U) & ~(size_t)15;
    if (!heap_ptr || (size_t)(heap_end - heap_ptr) < n) {
        heap_ptr = malloc(CHUNK);
        if (!heap_ptr) { fprintf(stderr, "sem memoria\n"); exit(2); }
        heap_end = heap_ptr + CHUNK;
    }
    void *p = heap_ptr; heap_ptr += n; n_alloc += n; return p;
}
static L mk(int type) { L o = halloc(sizeof(Obj)); o->type = type; return o; }
static L cons(L a, L d) { L o = mk(T_CONS); o->u.c.car = a; o->u.c.cdr = d; return o; }
static L mkfix(long n) { L o = mk(T_FIX); o->u.fix = n; return o; }
static L mkflo(double x) { L o = mk(T_FLO); o->u.flo = x; return o; }

#define CONSP(x) ((x)->type == T_CONS)
#define SYMP(x)  ((x)->type == T_SYM)
#define FIXP(x)  ((x)->type == T_FIX)
#define FLOP(x)  ((x)->type == T_FLO)
#define NUMP(x)  ((x)->type == T_FIX || (x)->type == T_FLO)
#define ATOMP(x) (!CONSP(x))

/* impressao limitada, sem alocacoes Lisp nem chamada ao avaliador */
static void f22_view(FILE *fp, L v, int depth) {
    if (!v) { fputs("#<NULL>", fp); return; }
    if (v == NIL) { fputs("NIL", fp); return; }
    if (SYMP(v)) { fputs(v->u.s.name, fp); return; }
    if (FIXP(v)) { fprintf(fp, "%ld", v->u.fix); return; }
    if (FLOP(v)) { fprintf(fp, "%.8g", v->u.flo); return; }
    if (!CONSP(v)) { fputs("#<FUNCTION>", fp); return; }
    if (depth >= 3) { fputs("(...)" , fp); return; }
    fputc('(', fp);
    L q=v;
    for (int i=0; i<5 && CONSP(q); ++i) {
        if (i) fputc(' ', fp);
        f22_view(fp, q->u.c.car, depth+1);
        q=q->u.c.cdr;
    }
    if (CONSP(q)) fputs(" ...", fp);
    else if (q != NIL) { fputs(" . ",fp); f22_view(fp,q,depth+1); }
    fputc(')',fp);
}
static int f22_named(L s, const char *name) {
    return s && SYMP(s) && strcmp(s->u.s.name,name)==0;
}
static L f22_lookup(const char *name);
static int f22_watch_pattern_depth(L x, int depth) {
    if (f22_named(x, ":B10") || f22_named(x, ":B4")) return 1;
    if (depth >= 5 || !CONSP(x)) return 0;
    L p=x;
    for (int i=0; i<8 && CONSP(p); ++i) {
        if (f22_watch_pattern_depth(p->u.c.car, depth+1)) return 1;
        p=p->u.c.cdr;
    }
    return p != NIL && !CONSP(p) && f22_watch_pattern_depth(p,depth+1);
}
static int f22_watch_pattern(L x) {
    return f22_watch_pattern_depth(x,0);
}
static void f22_record(const char *event, L x, L prop, L value) {
    if (!who_trace_fp) return;
    L who=f22_lookup("WHO"), last=f22_lookup("LASTSENTNO");
    fprintf(who_trace_fp,"%llu\t%ld\t%s\tobj=",++who_trace_events,n_evals,event);
    f22_view(who_trace_fp,x,0);
    fputs("\tprop=",who_trace_fp); f22_view(who_trace_fp,prop,0);
    fputs("\tresult=",who_trace_fp); f22_view(who_trace_fp,value,0);
    fputs("\tWHO=",who_trace_fp);
    f22_view(who_trace_fp,who ? (who->u.s.valcell ? who->u.s.valcell->u.c.car : NULL) : NULL,0);
    fputs("\tLASTSENTNO=",who_trace_fp);
    f22_view(who_trace_fp,last ? (last->u.s.valcell ? last->u.s.valcell->u.c.car : NULL) : NULL,0);
    fputc('\n',who_trace_fp);
}

/* F23: nenhuma ramificacao de resultado depende do rastreador. */
static int f23_tracked_callee(L x) {
    if (!x || !SYMP(x)) return 0;
    const char *name=x->u.s.name;
    static const char *const names[]={
        "THADD", "THIP", "THMATCHDB", "THMATCH1", "THTRY1",
        "THPUTPROP", "THREMOVE", "THTRUE", "WHO", "THMUNG",
        "THRECOMMEND", "THGOAL", "THMATCHLIST", "THMATCH"};
    for (size_t i=0;i<sizeof names/sizeof names[0];++i)
        if (strcmp(name,names[i])==0) return 1;
    return 0;
}
static void f23_event(const char *op, L obj, L val) {
    if (!f23_db_fp) return;
    L who=f22_lookup("WHO"), thwh=f22_lookup("THWH"), last=f22_lookup("LASTSENTNO");
    fprintf(f23_db_fp,"%llu\t%ld\t%s\tobj=",++f23_db_events,n_evals,op);
    f22_view(f23_db_fp,obj,0);
    fputs("\tvalue=",f23_db_fp);f22_view(f23_db_fp,val,0);
    fputs("\tWHO=",f23_db_fp);
    f22_view(f23_db_fp,who && who->u.s.valcell ? who->u.s.valcell->u.c.car : NULL,0);
    fputs("\tTHWH=",f23_db_fp);
    f22_view(f23_db_fp,thwh && thwh->u.s.valcell ? thwh->u.s.valcell->u.c.car : NULL,0);
    fputs("\tLASTSENTNO=",f23_db_fp);
    f22_view(f23_db_fp,last && last->u.s.valcell ? last->u.s.valcell->u.c.car : NULL,0);
    fputc('\n',f23_db_fp);
    /* O harness PTY encerra o processo ao final: descarregar cada evento. */
    fflush(f23_db_fp);
}

static L callstack[20000]; static int csp;
/* F24: ID sequencial por PRIMEIRA OBSERVACAO. Nunca serializa endereco bruto.
 * Mapa de ponteiros com enderecamento aberto, disponivel apenas durante trace.
 * IDs NAO sao IDs de alocacao, nem servem para inferir causalidade sozinhos. */
struct f24_slot { L key; unsigned long long id; };
static struct f24_slot *f24_slots;
static size_t f24_cap, f24_used;
static unsigned long long f24_serial;
static size_t f24_hash_ptr(L x) {
    uintptr_t u=(uintptr_t)x;
    u ^= u >> 15;
    u *= (uintptr_t)0x9e3779b1U;
    u ^= u >> 11;
    return (size_t)u;
}
static void f24_grow(void) {
    size_t n=f24_cap ? f24_cap*2U : 1024U;
    if (n <= f24_cap || n > ((size_t)-1)/sizeof(struct f24_slot)) {
        fputs("F24 TRACE OBJECT TABLE CAPACITY ERROR\n",stderr); exit(2);
    }
    struct f24_slot *next=calloc(n,sizeof *next);
    if (!next) { fputs("F24 TRACE OBJECT TABLE OUT OF MEMORY\n",stderr); exit(2); }
    for (size_t i=0;i<f24_cap;i++) if (f24_slots[i].key) {
        size_t k=f24_hash_ptr(f24_slots[i].key)&(n-1U);
        while(next[k].key) k=(k+1U)&(n-1U);
        next[k]=f24_slots[i];
    }
    free(f24_slots);f24_slots=next;f24_cap=n;
}
static unsigned long long f24_id(L obj) {
    if (!obj) return 0;
    if (f24_cap==0 || (f24_used+1U)*10U>=f24_cap*7U) f24_grow();
    size_t k=f24_hash_ptr(obj)&(f24_cap-1U);
    while(f24_slots[k].key && f24_slots[k].key!=obj) k=(k+1U)&(f24_cap-1U);
    if (f24_slots[k].key) return f24_slots[k].id;
    f24_slots[k].key=obj;
    f24_slots[k].id=++f24_serial;
    f24_used++;
    return f24_slots[k].id;
}
static int f24_target_fact(L x) {
    if (!x || !CONSP(x) || !CONSP(x->u.c.car)) return 0;
    L a=x->u.c.car;
    return f22_named(a->u.c.car,"#MANIP") &&
        CONSP(a->u.c.cdr) &&
        (f22_named(a->u.c.cdr->u.c.car,":B10") ||
         f22_named(a->u.c.cdr->u.c.car,":B4"));
}
static L f24_locate_fact(L root,int depth) {
    if (!root || !CONSP(root) || depth>5) return NULL;
    if (f24_target_fact(root)) return root;
    L x=root;
    for (int i=0;i<6 && x && CONSP(x);i++,x=x->u.c.cdr) {
        L got=f24_locate_fact(x->u.c.car,depth+1);
        if (got) return got;
    }
    return NULL;
}
/* Marcadores de ID nao afetam semantica: f22_view so le dados. */
static void f24_event(const char *op,L x,L v) {
    if (!f24_fp) return;
    L fact=f24_locate_fact(x,0);
    if (!fact) fact=f24_locate_fact(v,0);
    L who=f22_lookup("WHO"), last=f22_lookup("LASTSENTNO");
    L actor=(csp>1 ? callstack[csp-2] : NULL);
    /* F24: cada atribuicao de ID acontece em ordem definida pelo C99.
       Nao chamar f24_id varias vezes nos argumentos de um unico fprintf. */
    unsigned long long xid=f24_id(x);
    unsigned long long vid=f24_id(v);
    unsigned long long fid=f24_id(fact);
    unsigned long long carid=x && CONSP(x) ? f24_id(x->u.c.car) : 0;
    unsigned long long cdrid=x && CONSP(x) ? f24_id(x->u.c.cdr) : 0;
    unsigned long long factcarid=fact ? f24_id(fact->u.c.car) : 0;
    unsigned long long factcdrid=fact ? f24_id(fact->u.c.cdr) : 0;
    fprintf(f24_fp,"%llu\teval=%ld\tevent=%s\tobj_id=%llu\tval_id=%llu\tfact_id=%llu",
            ++f24_events,n_evals,op,xid,vid,fid);
    fprintf(f24_fp,"\tcar_id=%llu\tcdr_id=%llu\tfact_car_id=%llu\tfact_cdr_id=%llu",
            carid,cdrid,factcarid,factcdrid);
    fprintf(f24_fp,"\tdepth=%d\tcaller=",csp);
    f22_view(f24_fp,actor,0);
    fputs("\tobj=",f24_fp);f22_view(f24_fp,x,0);
    fputs("\tval=",f24_fp);f22_view(f24_fp,v,0);
    fputs("\tfact=",f24_fp);f22_view(f24_fp,fact,0);
    fputs("\tWHO=",f24_fp);
    f22_view(f24_fp,who && who->u.s.valcell ? who->u.s.valcell->u.c.car : NULL,0);
    fputs("\tLASTSENTNO=",f24_fp);
    f22_view(f24_fp,last && last->u.s.valcell ? last->u.s.valcell->u.c.car : NULL,0);
    fputc('\n',f24_fp);fflush(f24_fp);
}

/* ───────────────────────── símbolos ───────────────────────── */
#define HSIZE 65536
static struct hent { char *name; L sym; struct hent *next; } *htab[HSIZE];
static char *c99_strdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *p = malloc(n);
    if (!p) { fprintf(stderr, "sem memoria\n"); exit(2); }
    memcpy(p, s, n); return p;
}
static unsigned hash(const char *s) { unsigned h = 5381; while (*s) h = h * 33 + (unsigned char)*s++; return h & (HSIZE - 1); }
/* F14R: o nome imprimivel nao estabelece identidade; atomos nao internados
 * possuem plist/valor proprios e NAO entram no obarray. */
static L make_uninterned_symbol(const char *name) {
    L s = mk(T_SYM);
    s->u.s.name = c99_strdup(name);
    s->u.s.plist = NIL;
    s->u.s.valcell = NULL;
    s->u.s.macro = 0;
    return s;
}
static L intern(const char *name) {
    unsigned h = hash(name);
    for (struct hent *e = htab[h]; e; e = e->next) if (!strcmp(e->name, name)) return e->sym;
    L s = make_uninterned_symbol(name);
    struct hent *e = malloc(sizeof *e);
    if (!e) { fprintf(stderr, "sem memoria\n"); exit(2); }
    e->name = s->u.s.name; e->sym = s; e->next = htab[h]; htab[h] = e;
    return s;
}
static L f22_lookup(const char *name) {
    unsigned h=hash(name);
    for (struct hent *e=htab[h]; e; e=e->next)
        if (!strcmp(e->name,name)) return e->sym;
    return NULL;
}
static L gensym_counter;
static char gensym_prefix = 'G';
static const char *symname(L s) { return s->u.s.name; }

/* ───────────────────────── sinais (GO, RETURN, ERR) ───────────────────────── */
enum { SIG_NONE = 0, SIG_GO, SIG_RETURN, SIG_ERR, SIG_THROW };
static int sig;            /* sinal pendente */
static L sig_tag, sig_val; /* rótulo do GO / valor do RETURN ou ERR */
static int err_quiet;      /* ERR do usuário sem mensagem */
static char err_msg[512];
static int errset_depth, errset_silent;
static long n_errors;
static L err_obj;

static void print_backtrace(void);
static L lisp_error(const char *msg, L obj) {
    if (sig) return NIL;
    sig = SIG_ERR; sig_val = NIL; err_quiet = 0; err_obj = obj;
    snprintf(err_msg, sizeof err_msg, "%s", msg);
    n_errors++;
    print_backtrace();
    return NIL;
}
#define CHECK(x) do { if (sig) return NIL; (void)(x); } while (0)

/* ───────────────────────── valores e ligação ───────────────────────── */
static L getprop_cell(L sym, L ind) {              /* a célula (val . resto) da propriedade, ou NULL */
    for (L p = sym->u.s.plist; CONSP(p) && CONSP(p->u.c.cdr); p = p->u.c.cdr->u.c.cdr)
        if (p->u.c.car == ind) return p->u.c.cdr;
    return NULL;
}

static L remprop(L sym, L ind);
static void putprop(L sym, L val, L ind) {
    if (who_trace_fp && f22_named(sym,"THTRUE") && f22_named(ind,"EXPR"))
        f22_record("PUT_THTRUE_EXPR",sym,ind,val);
    /* MacLisp PUTPROP: ao redefinir, remover a entrada antiga e inseri-la na frente.
       Isto torna a definicao funcional mais recente a primeira na plist. */
    if (getprop_cell(sym, ind)) remprop(sym, ind);
    sym->u.s.plist = cons(ind, cons(val, sym->u.s.plist));
    if (ind == S_VALUE) sym->u.s.valcell = sym->u.s.plist->u.c.cdr;
}
static L remprop(L sym, L ind) {
    if (who_trace_fp && f22_named(sym,"THTRUE") && f22_named(ind,"EXPR"))
        f22_record("REMOVE_THTRUE_EXPR",sym,ind,NULL);
    L prev = NULL;
    for (L p = sym->u.s.plist; CONSP(p) && CONSP(p->u.c.cdr); prev = p->u.c.cdr, p = p->u.c.cdr->u.c.cdr)
        if (p->u.c.car == ind) {
            if (prev) prev->u.c.cdr = p->u.c.cdr->u.c.cdr; else sym->u.s.plist = p->u.c.cdr->u.c.cdr;
            if (ind == S_VALUE) sym->u.s.valcell = NULL;
            return T_;
        }
    return NIL;
}
static L symval(L s) {                              /* valor ou UNBOUND */
    if (s == NIL) return NIL;
    if (s->u.s.valcell) return s->u.s.valcell->u.c.car;
    return UNBOUND;
}
static void setval(L s, L v) {
    if (who_trace_fp && f22_named(s,"WHO")) f22_record("SET_WHO",s,NULL,v);
    if (s->u.s.valcell) s->u.s.valcell->u.c.car = v;
    else putprop(s, v, S_VALUE);
}
/* pilha de ligações dinâmicas */
static struct bind { L sym, old; } *bstack; static int bsp, bcap;
static void bind(L sym, L val) {
    if (bsp == bcap) { bcap = bcap ? bcap * 2 : 4096; bstack = realloc(bstack, bcap * sizeof *bstack); }
    bstack[bsp].sym = sym; bstack[bsp].old = symval(sym); bsp++;
    setval(sym, val);
}
static void unbind_to(int mark) {
    while (bsp > mark) {
        bsp--;
        L s = bstack[bsp].sym, old = bstack[bsp].old;
        if (old == UNBOUND) remprop(s, S_VALUE); else setval(s, old);
    }
}

/* ───────────────────────── listas básicas ───────────────────────── */
static L car(L x) { if (CONSP(x)) return x->u.c.car; return NIL; }   /* o MacLisp de 1971 nao confere o tipo em CAR: (CAR 5) nao e erro */
static L cdr(L x) { if (CONSP(x)) return x->u.c.cdr; if (x == NIL) return NIL; if (SYMP(x)) return x->u.s.plist; return NIL; }
static int length(L x) { int n = 0; while (CONSP(x)) { n++; x = x->u.c.cdr; } return n; }
static L nreverse(L x) { L r = NIL; while (CONSP(x)) { L d = x->u.c.cdr; x->u.c.cdr = r; r = x; x = d; } return r; }
static L list1(L a) { return cons(a, NIL); }
static L list2(L a, L b) { return cons(a, cons(b, NIL)); }
static L list3(L a, L b, L c) { return cons(a, cons(b, cons(c, NIL))); }
static int eq(L a, L b) {
    if (a == b) return 1;
    if (FIXP(a) && FIXP(b)) return a->u.fix == b->u.fix;   /* fixnums pequenos eram EQ no PDP-10 */
    return 0;
}
static int equal(L a, L b) {
    if (eq(a, b)) return 1;
    if (CONSP(a) && CONSP(b)) return equal(a->u.c.car, b->u.c.car) && equal(a->u.c.cdr, b->u.c.cdr);
    if (FLOP(a) && FLOP(b)) return a->u.flo == b->u.flo;
    return 0;
}

/* ───────────────────────── saída ───────────────────────── */
static FILE *out;
static int column, lastc;
static long n_output;
static L S_CHRCT, S_LINEL;
static int linel = 72;
static void outc(int c) {
    fputc(c, out); n_output++;
    if (c == '\n') column = 0; else if (c != '\r') column++;
    lastc = c;
    if (S_CHRCT) { L l = symval(S_LINEL); if (l != UNBOUND && FIXP(l)) linel = (int)l->u.fix; setval(S_CHRCT, mkfix(linel - column)); }
}
static void outs(const char *s) { while (*s) outc(*s++); }
static void terpri(void) { outc('\n'); }

static int numbase(void) { L b = symval(S_BASE); return (b != UNBOUND && FIXP(b)) ? (int)b->u.fix : 8; }
static void print_fix(long n, int base) {
    char buf[80]; int i = 0, neg = n < 0; unsigned long u = neg ? -(unsigned long)n : (unsigned long)n;
    if (u == 0) buf[i++] = '0';
    while (u) { buf[i++] = "0123456789"[u % base]; u /= base; }
    if (neg) buf[i++] = '-';
    while (i) outc(buf[--i]);
    if (base == 10 && symval(S_NOPOINT) == NIL) outc('.');
}
static L parse_number(const char *tok, int *ok);
static int needs_slash(const char *name) {
    /* um nome que seria lido como número, ou que contém delimitadores */
    const char *p = name; int ok;
    if (!*p) return 1;
    for (; *p; p++) if (strchr(" \t\n\r()'/\"", *p) || (unsigned char)*p < 32) return 1;
    parse_number(name, &ok);
    return ok;
}
static void prin1(L x);
static void print_sym(L x, int slashify) {
    const char *name = symname(x);
    if (!slashify) { outs(name); return; }
    if (!needs_slash(name)) { outs(name); return; }
    int ok; parse_number(name, &ok);
    for (const char *p = name; *p; p++) {
        if (strchr(" \t\n\r()'/\"", *p) || (unsigned char)*p < 32 || (ok && p == name)) outc('/');
        outc(*p);
    }
}
static void print_obj(L x, int slashify) {
    if (sig) return;
    if (!x) { outs("#<NULL>"); return; }
    switch (x->type) {
    case T_SYM: print_sym(x, slashify); return;
    case T_FIX: print_fix(x->u.fix, numbase()); return;
    case T_FLO: { char b[64]; snprintf(b, sizeof b, "%g", x->u.flo); if (!strchr(b, '.') && !strchr(b, 'e')) strcat(b, ".0"); outs(b); return; }
    case T_SUBR: outs("#"); outs(x->u.subr.name); return;
    case T_CONS: {
        outc('(');
        for (;;) {
            print_obj(x->u.c.car, slashify);
            x = x->u.c.cdr;
            if (x == NIL) break;
            if (!CONSP(x)) { outs(" . "); print_obj(x, slashify); break; }
            outc(' ');
        }
        outc(')'); return;
    }
    }
}
static void prin1(L x) { print_obj(x, 1); }
static void princ(L x) { print_obj(x, 0); }

/* ───────────────────────── entrada ───────────────────────── */
typedef struct Stream { const char *buf; size_t pos, len; int pending_lf; const char *name; int line; int is_tty; } Stream;
static Stream *in;                 /* fluxo corrente */

static Stream *tty;                /* entrada do usuário (stdin ou arquivo de frases) */
static int echo_tty = 0;
static char *read_file(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb"); if (!f) return NULL;
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *b = malloc(n + 1); if (fread(b, 1, n, f) != (size_t)n) { fclose(f); free(b); return NULL; }
    b[n] = 0; fclose(f); *len = n; return b;
}
/* próximo caractere: '\n' do arquivo vira CR LF, como no ITS; -1 no fim */
static int tyi_raw(Stream *s) {
    if (s->pending_lf) { s->pending_lf = 0; return '\n'; }
    if (s->pos >= s->len) {
        if (s->is_tty) {                     /* ler mais uma linha do teclado */
            static char line[4096];
            fflush(out); if (!fgets(line, sizeof line, stdin)) return -1;
            size_t l = strlen(line);
            s->buf = c99_strdup(line); s->pos = 0; s->len = l;
            if (echo_tty) { fputs(line, out); fflush(out); }
        } else return -1;
    }
    int c = (unsigned char)s->buf[s->pos++];
    if (c == '\r' && s->pos < s->len && s->buf[s->pos] == '\n') { s->pos++; s->pending_lf = 1; s->line++; return '\r'; }
    if (c == '\n') { s->pending_lf = 1; s->line++; return '\r'; }
    if (c == 3) return -1;                                 /* ^C: fim de arquivo do ITS */
    return c;
}
static int tyi(void) { return tyi_raw(in); }
static int tyipeek(void) {
    Stream *s = in;
    if (s->pending_lf) return '\n';
    if (s->pos >= s->len) {
        if (!s->is_tty) return -1;
        int c = tyi_raw(s);                    /* força a leitura da próxima linha e devolve o char */
        if (c < 0) return -1;
        /* desfaz: */
        if (c == '\r') { s->pending_lf = 0; s->pos = 0; return '\r'; }
        s->pos--; return c;
    }
    int c = (unsigned char)s->buf[s->pos];
    if (c == '\n') return '\r';
    return c;
}
static L read_obj(void);
static int is_white(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == 0 || c == 11; }
static int is_delim(int c) { return c < 0 || is_white(c) || c == '(' || c == ')' || c == '\'' ; }
static L macro_table[256];        /* (SSTATUS MACRO char fn) */
static int macro_splice[256];
static L apply(L fn, L args);
static L eval(L x);

static L parse_number(const char *tok, int *ok) {
    /* inteiro na base IBASE; sufixo '.' = decimal; ponto no meio = flonum decimal */
    const char *p = tok; int neg = 0; *ok = 0;
    if (*p == '-' || *p == '+') { neg = *p == '-'; p++; }
    if (!*p) return NIL;
    const char *q = p; int digits = 0, dots = 0;
    for (; *q; q++) { if (isdigit((unsigned char)*q)) digits++; else if (*q == '.') dots++; else return NIL; }
    if (!digits) return NIL;
    L ib = symval(S_IBASE); int base = (ib != UNBOUND && FIXP(ib)) ? (int)ib->u.fix : 8;
    if (dots == 0 || (dots == 1 && q[-1] == '.')) {
        if (dots == 1) base = 10;
        /* F20: limites host-long; leitura nao pode provocar signed overflow
         * em n = n * base + d. Fora do intervalo gera erro explicito.
         * Bignums historicos ainda nao implementados. */
        unsigned long maxmag = neg ? (unsigned long)LONG_MAX + 1UL : (unsigned long)LONG_MAX;
        unsigned long mag = 0;
        if (base < 2 || base > 10) return NIL;
        for (const char *r = p; *r && *r != '.'; r++) {
            unsigned long d = (unsigned long)(*r - '0');
            if (d >= (unsigned long)base) return NIL;
            if (mag > (maxmag - d) / (unsigned long)base) {
                *ok = -1; return NIL;
            }
            mag = mag * (unsigned long)base + d;
        }
        *ok = 1;
        if (neg && mag == (unsigned long)LONG_MAX + 1UL) return mkfix(LONG_MIN);
        return mkfix(neg ? -(long)mag : (long)mag);
    }
    if (dots == 1) { *ok = 1; double x = atof(p); return mkflo(neg ? -x : x); }
    return NIL;
}
static L read_token(int first) {
    char buf[512]; int n = 0, esc = 0, c = first;
    for (;;) {
        if (c == '/') { c = tyi(); if (c < 0) break; esc = 1; if (n < 510) buf[n++] = c; }
        else { if (n < 510) buf[n++] = (char)(islower(c) ? toupper(c) : c); }
        c = tyipeek();
        if (c < 0 || is_delim(c) || (c < 256 && macro_table[c] && c != '\'')) break;
        c = tyi();
    }
    buf[n] = 0;
    if (!esc) {
        if (!strcmp(buf, ".")) return DOT_MARK;
        int ok; L num = parse_number(buf, &ok);
        if (ok < 0) return lisp_error("FIXNUM LITERAL OUT OF HOST-LONG RANGE", NIL);
        if (ok) return num;
    }
    return intern(buf);
}
static L EOF_OBJ;

static L read_list(void) {
    L head = NIL, tail = NULL;
    for (;;) {
        int c;
        do c = tyi(); while (c >= 0 && is_white(c));
        if (c < 0) return lisp_error("UNBALANCED PARENTHESES (fim do arquivo)", NIL);
        if (c == ')') return head;
        L x;
        if (c == '(') x = read_list();
        else if (c == '\'' && macro_table['\''] == NULL) x = list2(S_QUOTE, read_obj());
        else if (c < 256 && macro_table[c]) {
            L r = apply(macro_table[c], NIL); if (sig) return NIL;
            if (macro_splice[c]) { for (L p = r; CONSP(p); p = p->u.c.cdr) { L cell = list1(p->u.c.car); if (tail) tail->u.c.cdr = cell; else head = cell; tail = cell; } continue; }
            x = r;
        }
        else x = read_token(c);
        if (sig) return NIL;
        if (x == DOT_MARK && head != NIL) {
            L d = read_obj(); if (sig) return NIL;
            do c = tyi(); while (c >= 0 && is_white(c));
            if (c != ')') return lisp_error("BAD DOTTED PAIR", NIL);
            tail->u.c.cdr = d; return head;
        }
        L cell = list1(x);
        if (tail) tail->u.c.cdr = cell; else head = cell;
        tail = cell;
    }
}
static L read_obj(void) {
    for (;;) {
        int c;
        do c = tyi(); while (c >= 0 && is_white(c));
        if (c < 0) return EOF_OBJ;
        if (c == '(') return read_list();
        if (c == ')') continue;                         /* parêntese a mais: ignorado, como o MacLisp */
        if (c == '\'' && macro_table['\''] == NULL) { L x = read_obj(); if (x == EOF_OBJ) return x; return list2(S_QUOTE, x); }
        if (c < 256 && macro_table[c]) {
            L r = apply(macro_table[c], NIL); if (sig) return NIL;
            if (macro_splice[c]) { if (r == NIL) continue; return r->u.c.car; }
            return r;
        }
        return read_token(c);
    }
}

/* ───────────────────────── avaliador ───────────────────────── */
/* F21: LABEL cria ligacao funcional DINAMICA. Nao altera a plist global:
 * uma definicao recursiva local desaparece ao retornar, mesmo apos SIG_ERR.
 * As estruturas sao locais as chamadas ativas de APPLY, sem fuga de ponteiros. */
struct label_scope { L sym, definition; struct label_scope *prev; };
static struct label_scope *active_label;
static L findfn(L sym, L *kind) {               /* primeira propriedade funcional da plist */
    for (const struct label_scope *l = active_label; l; l = l->prev)
        if (l->sym == sym) { *kind = S_EXPR; return l->definition; }
    for (L p = sym->u.s.plist; CONSP(p) && CONSP(p->u.c.cdr); p = p->u.c.cdr->u.c.cdr) {
        L i = p->u.c.car;
        if (i == S_EXPR || i == S_FEXPR || i == S_SUBR || i == S_FSUBR || i == S_LSUBR || i == S_MACRO) { *kind = i; return p->u.c.cdr->u.c.car; }
    }
    return NULL;
}
static int depth;
/* F25: captura observacional apenas em LASTSENTNO=15 (turno 16).
 * Nunca altera valores, propriedades, escolhas de funcao ou THALIST.
 * Usa findfn exatamente como o avaliador, para registrar a definicao
 * efetivamente selecionada de THTRUE, incluindo prioridade SUBR/EXPR. */
static L f25_value(const char *name) {
    L s=f22_lookup(name);
    return s ? symval(s) : NULL;
}
static int f25_focus(L name) {
    if (!name || !SYMP(name)) return 0;
    const char *p=name->u.s.name;
    static const char *const targets[]={
        "WHO","THTRUE","THTRY1","THMATCH1","THMATCHDB","THMATCHLIST",
        "THGOALF","THGOAL","THBIND","THAPPLY1","THTRY","THRECOMMEND",
        "THRETURN","THMATCH","THFINDT","THFINDF"
    };
    for (size_t i=0;i<sizeof targets/sizeof targets[0];i++)
        if (!strcmp(p,targets[i])) return 1;
    return 0;
}
static void f25_trace(const char *phase, L name, L args, L value) {
    if (!f25_fp) return;
    L last=f25_value("LASTSENTNO");
    if (!last || !FIXP(last) || last->u.fix!=15) return;
    L s=f22_lookup("THTRUE"), kind=NULL;
    L selected=s ? findfn(s,&kind) : NULL;
    L expr=s ? getprop_cell(s,S_EXPR) : NULL;
    L subr=s ? getprop_cell(s,S_SUBR) : NULL;
    fprintf(f25_fp,"%llu\teval=%ld\tphase=%s\tfn=",++f25_events,n_evals,phase);
    f22_view(f25_fp,name,0);
    fprintf(f25_fp,"\tdepth=%d\tcall_depth=%d\targs=",depth,csp);
    f22_view(f25_fp,args,0);
    fputs("\tresult=",f25_fp);f22_view(f25_fp,value,0);
    fputs("\tWHO=",f25_fp);f22_view(f25_fp,f25_value("WHO"),0);
    fputs("\tTHALIST=",f25_fp);f22_view(f25_fp,f25_value("THALIST"),0);
    fputs("\tTHOLIST=",f25_fp);f22_view(f25_fp,f25_value("THOLIST"),0);
    fputs("\tTHBRANCH=",f25_fp);f22_view(f25_fp,f25_value("THBRANCH"),0);
    fputs("\tTHTRUE_KIND=",f25_fp);f22_view(f25_fp,kind,0);
    fputs("\tTHTRUE_SELECTED=",f25_fp);f22_view(f25_fp,selected,0);
    fputs("\tTHTRUE_EXPR=",f25_fp);f22_view(f25_fp,expr?expr->u.c.car:NIL,0);
    fputs("\tTHTRUE_SUBR=",f25_fp);f22_view(f25_fp,subr?subr->u.c.car:NIL,0);
    fputc('\n',f25_fp);
    fflush(f25_fp); /* o harness PTY encerra o filho ao final */
}

static int trace_errors = 0;
static void print_backtrace(void) {
    if (!trace_errors) return;
    fprintf(stderr, "  pilha:");
    int n = 0; for (int i = csp - 1; i >= 0 && n < 25; i--, n++) fprintf(stderr, " %s", symname(callstack[i]));
    fprintf(stderr, "\n");
}
static L lexpr_args[64]; static int lexpr_n[64]; static int lexpr_sp;  /* argumentos das LEXPRs ativas */
/* F20: observacao opcional, sem alterar despacho ou preenchimento dos params.
 * Nao e uma implementacao da interrupcao historica wrng-no-args.
 * -A: imprime ate 64 chamadas EXPR com aridade discordante em stderr. */
static int trace_arity;
static unsigned long long arity_mismatches;
static int trace_arity_printed;


static L evlis(L args) {
    L head = NIL, tail = NULL;
    for (; CONSP(args); args = args->u.c.cdr) {
        L v = eval(args->u.c.car); if (sig) return NIL;
        L cell = list1(v); if (tail) tail->u.c.cdr = cell; else head = cell; tail = cell;
    }
    return head;
}
static L progn(L body) { L v = NIL; for (; CONSP(body); body = body->u.c.cdr) { v = eval(body->u.c.car); if (sig) return NIL; } return v; }

static L apply_lambda(L fn, L args, int evaluated_args_are_list) {
    /* fn = (LAMBDA params . body); args = lista de valores (ou, para FEXPR, o argumento único) */
    L params = car(cdr(fn)); L body = cdr(cdr(fn));
    int mark = bsp;
    if (SYMP(params) && params != NIL) {               /* LEXPR: (LAMBDA N ...) */
        /* F16: a pilha de argumentos LEXPR tem 64 slots. No original F15,
           lexpr_sp podia crescer alem desse limite; ARG/SETARG/LISTIFY
           acessariam fora dos vetores. Falha explicita, sem corromper a pilha. */
        if (lexpr_sp >= (int)(sizeof lexpr_args / sizeof lexpr_args[0]))
            return lisp_error("LEXPR STACK OVERFLOW", fn);
        int n = length(args);
        bind(params, mkfix(n));
        lexpr_args[lexpr_sp] = args;
        lexpr_n[lexpr_sp] = n;
        lexpr_sp++;
        L v = progn(body);
        lexpr_sp--;
        unbind_to(mark);
        if (sig == SIG_RETURN) { sig = SIG_NONE; v = sig_val; }   /* RETURN fora de PROG devolve da função (tolerante) */
        return v;
    }
    /* F20: diagnostico nao intrusivo. Nao promover validacao estrita
     * enquanto a prova historica de EXPR/FEXPR/compilados estiver pendente. */
    if (trace_arity && evaluated_args_are_list) {
        int expected = length(params), observed = length(args);
        if (expected != observed) {
            arity_mismatches++;
            if (trace_arity_printed < 64) {
                const char *callee = (csp && SYMP(callstack[csp-1])) ?
                    symname(callstack[csp-1]) : "<APPLY>";
                fprintf(stderr,
                        ";ARITY_DIAG eval=%ld callee=%s expected=%d got=%d\n",
                        n_evals, callee, expected, observed);
                trace_arity_printed++;
            }
        }
    }
    L p = params, a = args;
    for (; CONSP(p); p = p->u.c.cdr) {
        L v = CONSP(a) ? a->u.c.car : NIL;
        if (CONSP(a)) a = a->u.c.cdr;
        bind(p->u.c.car, v);
    }
    (void)evaluated_args_are_list;
    /* F23: argumentos ja avaliados, capturados antes do corpo mutar estruturas. */
    L f23_callee = csp>0 ? callstack[csp-1] : NULL;
    int f23_call = f23_db_fp && f23_tracked_callee(f23_callee) && f22_watch_pattern(args);
    if (f23_call) f23_event("CALL",f23_callee,args);
    if (f24_fp && f23_tracked_callee(f23_callee) && f22_watch_pattern(args)) f24_event("CALL",f23_callee,args);
    if (f25_fp && f25_focus(f23_callee)) f25_trace("APPLY_ENTER",f23_callee,args,NULL);
    L v = progn(body);
    if (f25_fp && f25_focus(f23_callee)) f25_trace("APPLY_EXIT",f23_callee,args,v);
    if (f23_call) f23_event("RETURN",f23_callee,v);
    if (f24_fp && f23_tracked_callee(f23_callee) && f22_watch_pattern(args)) f24_event("RETURN",f23_callee,v);
    unbind_to(mark);
    return v;
}
static L call_subr(L sub, L args) {
    int kind = sub->u.subr.kind;
    int n = length(args);
    if (kind == K_SUBR) {
        if (n < sub->u.subr.min) {
            char m[128]; snprintf(m, sizeof m, "%s TOO FEW ARGUMENTS", sub->u.subr.name);
            return lisp_error(m, args);
        }
        if (sub->u.subr.max >= 0 && n > sub->u.subr.max) {
            char m[128]; snprintf(m, sizeof m, "%s TOO MANY ARGUMENTS", sub->u.subr.name);
            return lisp_error(m, args);
        }
    }
    if (kind == K_LSUBR || kind == K_FSUBR) return sub->u.subr.fn.f1(args);
    L a[4] = { NIL, NIL, NIL, NIL };
    int i = 0;
    for (L p = args; CONSP(p) && i < 4; p = p->u.c.cdr) a[i++] = p->u.c.car;
    /* The C arity is independent of Lisp's allowed input count. */
    switch (sub->u.subr.arity) {
    case 0: return sub->u.subr.fn.f0();
    case 1: return sub->u.subr.fn.f1(a[0]);
    case 2: return sub->u.subr.fn.f2(a[0], a[1]);
    case 3: return sub->u.subr.fn.f3(a[0], a[1], a[2]);
    case 4: return sub->u.subr.fn.f4(a[0], a[1], a[2], a[3]);
    default: return lisp_error("UNSUPPORTED SUBR ARITY", sub);
    }
}
/* aplica uma função a argumentos JÁ avaliados */
static L apply(L fn, L args) {
    if (sig) return NIL;
    if (SYMP(fn)) {
        L kind, def = findfn(fn, &kind);
        if (!def) {
            L v = symval(fn);                           /* (APPLY 'X ...) com X valendo uma função */
            if (v != UNBOUND && v != NIL && !SYMP(v)) return apply(v, args);
            char m[128]; snprintf(m, sizeof m, "%s UNDEFINED FUNCTION", symname(fn)); return lisp_error(m, fn);
        }
        if (kind == S_EXPR && SYMP(def) && def != NIL) return apply(def, args);   /* (DEFPROP THTRUE WHO EXPR): a definicao e outro nome de funcao */
        if (kind == S_EXPR) return apply_lambda(def, args, 1);
        if ((kind == S_SUBR || kind == S_LSUBR) && CONSP(def)) return apply_lambda(def, args, 1);   /* compilada na imagem: guardamos o LAMBDA */
        if ((kind == S_FSUBR) && CONSP(def)) return apply_lambda(def, list1(args), 0);
        if (kind == S_SUBR || kind == S_LSUBR) return call_subr(def, args);
        if (kind == S_FEXPR) return apply_lambda(def, list1(args), 0);   /* APPLY de FEXPR: recebe a lista */
        if (kind == S_FSUBR) return call_subr(def, args);
        if (kind == S_MACRO) { L form = cons(fn, args); L exp = apply_lambda(def, list1(form), 0); if (sig) return NIL; return eval(exp); }
    }
    if (CONSP(fn)) {
        if (fn->u.c.car == S_LAMBDA) return apply_lambda(fn, args, 1);
        if (fn->u.c.car == S_FUNCTION || fn->u.c.car == S_QUOTE) return apply(car(cdr(fn)), args);
        if (fn->u.c.car == intern("LABEL")) {
            L name = car(cdr(fn)), lam = car(cdr(cdr(fn)));
            if (!SYMP(name) || name == NIL || !CONSP(lam) || lam->u.c.car != S_LAMBDA)
                return lisp_error("LABEL REQUIRES NAME AND LAMBDA", fn);
            struct label_scope frame = { name, lam, active_label };
            active_label = &frame;
            L v = apply_lambda(lam, args, 1);
            active_label = frame.prev;
            return v;
        }
        /* (FUNARG ...) e outros: tenta avaliar a cabeça */
        L f = eval(fn); if (sig) return NIL; return apply(f, args);
    }
    if (fn->type == T_SUBR) return call_subr(fn, args);
    return lisp_error("NOT A FUNCTION", fn);
}
static L eval(L x) {
    if (sig) return NIL;
    n_evals++;
    if (!CONSP(x)) {
        if (SYMP(x)) {
            if (x == NIL) return NIL;
            L v = symval(x);
            if (v == UNBOUND) { char m[128]; snprintf(m, sizeof m, "%s UNBOUND VARIABLE", symname(x)); return lisp_error(m, x); }
            return v;
        }
        return x;                                     /* números */
    }
    if (++depth > 20000) { depth = 0; return lisp_error("PDL OVERFLOW", x); }
    L head = x->u.c.car, args = x->u.c.cdr, v = NIL;
    int saved_csp = csp;
    if (SYMP(head)) {
        if (csp < 20000) callstack[csp++] = head;
        L kind, def = findfn(head, &kind);
        int f25_this = f25_fp && f25_focus(head);
        if (f25_this) f25_trace("EVAL_DISPATCH",head,args,def);
        if (f24_fp && f22_named(head,"THTRUE") && f22_watch_pattern(args))
            f24_event("THTRUE_DISPATCH",head,def);
        if (!def) {
            L sv = symval(head);
            if (sv != UNBOUND && CONSP(sv) && sv->u.c.car == S_LAMBDA) { L a = evlis(args); if (!sig) v = apply_lambda(sv, a, 1); }
            else if (sv != UNBOUND && sv != NIL && SYMP(sv) && findfn(sv, &kind)) { L a = evlis(args); if (!sig) v = apply(sv, a); }   /* (FN X) com FN valendo CAR */
            else { char m[128]; snprintf(m, sizeof m, "%s UNDEFINED FUNCTION", symname(head)); v = lisp_error(m, x); }
        }
        else if ((kind == S_SUBR || kind == S_LSUBR) && CONSP(def)) { L a = evlis(args); if (!sig) v = apply_lambda(def, a, 1); }
        else if (kind == S_FSUBR && CONSP(def)) v = apply_lambda(def, list1(args), 0);
        else if (kind == S_SUBR || kind == S_LSUBR) { L a = evlis(args); if (!sig) v = call_subr(def, a); }
        else if (kind == S_FSUBR) v = call_subr(def, args);
        else if (kind == S_EXPR && SYMP(def) && def != NIL) { L a = evlis(args); if (!sig) v = apply(def, a); }
        else if (kind == S_EXPR) { L a = evlis(args); if (!sig) v = apply_lambda(def, a, 1); }
        else if (kind == S_FEXPR) v = apply_lambda(def, list1(args), 0);
        else if (kind == S_MACRO) { L exp = apply_lambda(def, list1(x), 0); if (!sig) v = eval(exp); }
        if (f25_this) f25_trace("EVAL_RETURN",head,args,v);
    } else if (CONSP(head) && head->u.c.car == S_LAMBDA) {
        L a = evlis(args); if (!sig) v = apply_lambda(head, a, 1);
    } else if (CONSP(head) && head->u.c.car == intern("LABEL")) {
        /* F21: LABEL e forma funcional, tambem valida diretamente no CAR
         * de uma aplicacao, nao apenas como argumento de APPLY. */
        L a = evlis(args); if (!sig) v = apply(head, a);
    } else {
        L f = eval(head); if (!sig) { L a = evlis(args); if (!sig) v = apply(f, a); }
    }
    depth--; csp = saved_csp;
    return v;
}

/* ───────────────────────── primitivas: utilitários ───────────────────────── */
static L B(int b) { return b ? T_ : NIL; }
/* C99 strictly typed registrars: never store function pointers as void*. */
typedef L (*subrfn0)(void);
typedef L (*subrfn1)(L);
typedef L (*subrfn2)(L,L);
typedef L (*subrfn3)(L,L,L);
typedef L (*subrfn4)(L,L,L,L);
#define DEFREG(N, TYPE, FIELD) \
static void defsubr##N(const char *name, TYPE fn, int min, int max) { \
    L o = mk(T_SUBR); o->u.subr.name = name; o->u.subr.kind = K_SUBR; \
    o->u.subr.min = min; o->u.subr.max = max; o->u.subr.arity = N; \
    o->u.subr.fn.FIELD = fn; putprop(intern(name), o, S_SUBR); \
}
DEFREG(0, subrfn0, f0)
DEFREG(1, subrfn1, f1)
DEFREG(2, subrfn2, f2)
DEFREG(3, subrfn3, f3)
#undef DEFREG
static void deflistfn(const char *name, L (*fn)(L), int kind) {
    L o = mk(T_SUBR); o->u.subr.name = name; o->u.subr.kind = kind;
    o->u.subr.min = 0; o->u.subr.max = -1; o->u.subr.arity = 1;
    o->u.subr.fn.f1 = fn;
    putprop(intern(name), o, kind == K_FSUBR ? S_FSUBR : S_LSUBR);
}
static void deffsubr(const char *name, L (*fn)(L)) { deflistfn(name, fn, K_FSUBR); }
static void deflsubr(const char *name, L (*fn)(L)) { deflistfn(name, fn, K_LSUBR); }
static long fixval(L x, const char *who) {
    if (FIXP(x)) return x->u.fix;
    if (FLOP(x)) return (long)x->u.flo;
    char m[128]; snprintf(m, sizeof m, "%s NON-NUMERIC ARG", who); lisp_error(m, x); return 0;
}
static double floval(L x) { return FIXP(x) ? (double)x->u.fix : FLOP(x) ? x->u.flo : 0.0; }

/* ── listas ── */
static L f_car(L x) { return car(x); }
static L f_cdr(L x) { return cdr(x); }
#define CXR(name, ops) static L name(L x) { const char *o = ops; for (int i = (int)strlen(o) - 1; i >= 0 && !sig; i--) x = o[i] == 'A' ? car(x) : cdr(x); return x; }
CXR(f_caar, "AA") CXR(f_cadr, "AD") CXR(f_cdar, "DA") CXR(f_cddr, "DD")
CXR(f_caaar, "AAA") CXR(f_caadr, "AAD") CXR(f_cadar, "ADA") CXR(f_caddr, "ADD") CXR(f_cdaar, "DAA") CXR(f_cdadr, "DAD") CXR(f_cddar, "DDA") CXR(f_cdddr, "DDD")
CXR(f_caaaar, "AAAA") CXR(f_caaadr, "AAAD") CXR(f_caadar, "AADA") CXR(f_caaddr, "AADD") CXR(f_cadaar, "ADAA") CXR(f_cadadr, "ADAD") CXR(f_caddar, "ADDA") CXR(f_cadddr, "ADDD")
CXR(f_cdaaar, "DAAA") CXR(f_cdaadr, "DAAD") CXR(f_cdadar, "DADA") CXR(f_cdaddr, "DADD") CXR(f_cddaar, "DDAA") CXR(f_cddadr, "DDAD") CXR(f_cdddar, "DDDA") CXR(f_cddddr, "DDDD")
static L f_cons(L a, L d) { return cons(a, d); }
static L f_ncons(L a) { return cons(a, NIL); }
static L f_xcons(L d, L a) { return cons(a, d); }
static L f_list(L args) { return args; }
static L f_append(L args) {
    L head = NIL, tail = NULL;
    for (L a = args; CONSP(a); a = a->u.c.cdr) {
        if (!CONSP(a->u.c.cdr)) { if (tail) tail->u.c.cdr = a->u.c.car; else head = a->u.c.car; break; }
        for (L p = a->u.c.car; CONSP(p); p = p->u.c.cdr) { L cell = list1(p->u.c.car); if (tail) tail->u.c.cdr = cell; else head = cell; tail = cell; }
    }
    return head;
}
static L f_nconc(L args) {
    L head = NIL, last = NULL;
    for (L a = args; CONSP(a); a = a->u.c.cdr) {
        L x = a->u.c.car; if (!CONSP(x)) { if (!CONSP(a->u.c.cdr) && last) last->u.c.cdr = x; continue; }
        if (last) last->u.c.cdr = x; else head = x;
        last = x; while (CONSP(last->u.c.cdr)) last = last->u.c.cdr;
    }
    return head;
}
static L f_reverse(L x) { L r = NIL; for (; CONSP(x); x = x->u.c.cdr) r = cons(x->u.c.car, r); return r; }
static L f_length(L x) { return mkfix(length(x)); }
static L f_last(L x) { if (!CONSP(x)) return x; while (CONSP(x->u.c.cdr)) x = x->u.c.cdr; return x; }
static L f_memq(L a, L l) { for (; CONSP(l); l = l->u.c.cdr) if (eq(a, l->u.c.car)) return l; return NIL; }
static L f_member(L a, L l) { for (; CONSP(l); l = l->u.c.cdr) if (equal(a, l->u.c.car)) return l; return NIL; }
static L f_assq(L a, L l) { for (; CONSP(l); l = l->u.c.cdr) if (CONSP(l->u.c.car) && eq(a, l->u.c.car->u.c.car)) return l->u.c.car; return NIL; }
static L f_assoc(L a, L l) { for (; CONSP(l); l = l->u.c.cdr) if (CONSP(l->u.c.car) && equal(a, l->u.c.car->u.c.car)) return l->u.c.car; return NIL; }
static L f_sassq(L a, L l, L fn) { L r = f_assq(a, l); if (r != NIL) return r; return apply(fn, NIL); }
static L f_sassoc(L a, L l, L fn) { L r = f_assoc(a, l); if (r != NIL) return r; return apply(fn, NIL); }
static L f_delq(L a, L l) { L head = l, prev = NULL; for (L p = l; CONSP(p); p = p->u.c.cdr) { if (eq(a, p->u.c.car)) { if (prev) prev->u.c.cdr = p->u.c.cdr; else head = p->u.c.cdr; } else prev = p; } return head; }
static L f_delete(L a, L l) { L head = l, prev = NULL; for (L p = l; CONSP(p); p = p->u.c.cdr) { if (equal(a, p->u.c.car)) { if (prev) prev->u.c.cdr = p->u.c.cdr; else head = p->u.c.cdr; } else prev = p; } return head; }
/* MacLisp de 1975: SUBST compara objetos com EQ e copia toda a estrutura.
 * Ao contrario de SUBLIS, mesmo partes nao substituidas sao copiadas. */
static L subst(L n, L o, L x) { if (eq(x, o)) return n; if (CONSP(x)) return cons(subst(n, o, x->u.c.car), subst(n, o, x->u.c.cdr)); return x; }
static L f_subst(L n, L o, L x) { return subst(n, o, x); }
/* F26: conserva identidade das subarvores nao alteradas.
 * BACKREF/PARAP/ FROM dependem de EQ entre sufixos compartilhados.
 * Criar uma celula nova apenas quando algum filho for efetivamente substituido.
 * Nao ha casos especiais para frases, turnos ou referencias historicas. */
static L sublis(L al, L x) {
    if (CONSP(x)) {
        L a = sublis(al, x->u.c.car);
        L d = sublis(al, x->u.c.cdr);
        if (a == x->u.c.car && d == x->u.c.cdr) return x;
        return cons(a, d);
    }
    L p = f_assq(x, al);
    return p != NIL ? p->u.c.cdr : x;
}
static L f_sublis(L al, L x) { return sublis(al, x); }
static L f_rplaca(L x, L v) {
    if (!CONSP(x)) return lisp_error("RPLACA WRONG TYPE ARG", x);
    int watch=f23_db_fp && (f22_watch_pattern(x) || f22_watch_pattern(v));
    int watch_f24=f24_fp && (f22_watch_pattern(x) || f22_watch_pattern(v));
    if (watch) f23_event("RPLACA_BEFORE",x,v);
    if (watch_f24) f24_event("RPLACA_BEFORE",x,v);
    x->u.c.car=v;
    if (watch) f23_event("RPLACA_AFTER",x,v);
    if (watch_f24) f24_event("RPLACA_AFTER",x,v);
    return x;
}
static L f_rplacd(L x, L v) {
    if (!CONSP(x)) return lisp_error("RPLACD WRONG TYPE ARG", x);
    int watch=f23_db_fp && (f22_watch_pattern(x) || f22_watch_pattern(v));
    int watch_f24=f24_fp && (f22_watch_pattern(x) || f22_watch_pattern(v));
    if (watch) f23_event("RPLACD_BEFORE",x,v);
    if (watch_f24) f24_event("RPLACD_BEFORE",x,v);
    x->u.c.cdr=v;
    if (watch) f23_event("RPLACD_AFTER",x,v);
    if (watch_f24) f24_event("RPLACD_AFTER",x,v);
    return x;
}
static L f_nreverse(L x) { return nreverse(x); }

/* ── predicados ── */
static L f_atom(L x) { return B(ATOMP(x)); }
static L f_null(L x) { return B(x == NIL); }
static L f_eq(L a, L b) { return B(eq(a, b)); }
static L f_equal(L a, L b) { return B(equal(a, b)); }
static L f_numberp(L x) { return B(NUMP(x)); }
static L f_fixp(L x) { return B(FIXP(x)); }
static L f_floatp(L x) { return B(FLOP(x)); }
static L f_boundp(L x) { return B(SYMP(x) && symval(x) != UNBOUND); }

/* ── aritmética ── */
static int anyflo(L args) { for (; CONSP(args); args = args->u.c.cdr) if (FLOP(args->u.c.car)) return 1; return 0; }
static L f_plus(L args) { if (anyflo(args)) { double s = 0; for (; CONSP(args); args = args->u.c.cdr) s += floval(args->u.c.car); return mkflo(s); } long s = 0; for (; CONSP(args); args = args->u.c.cdr) s += fixval(args->u.c.car, "PLUS"); return mkfix(s); }
static L f_times(L args) { if (anyflo(args)) { double s = 1; for (; CONSP(args); args = args->u.c.cdr) s *= floval(args->u.c.car); return mkflo(s); } long s = 1; for (; CONSP(args); args = args->u.c.cdr) s *= fixval(args->u.c.car, "TIMES"); return mkfix(s); }
static L f_difference(L args) {
    if (!CONSP(args)) return mkfix(0);
    if (anyflo(args)) { double s = floval(args->u.c.car); for (args = args->u.c.cdr; CONSP(args); args = args->u.c.cdr) s -= floval(args->u.c.car); return mkflo(s); }
    long s = fixval(args->u.c.car, "DIFFERENCE"); for (args = args->u.c.cdr; CONSP(args); args = args->u.c.cdr) s -= fixval(args->u.c.car, "DIFFERENCE"); return mkfix(s);
}
static L f_quotient(L args) {
    if (!CONSP(args)) return mkfix(1);
    if (anyflo(args)) { double s = floval(args->u.c.car); for (args = args->u.c.cdr; CONSP(args); args = args->u.c.cdr) s /= floval(args->u.c.car); return mkflo(s); }
    long s = fixval(args->u.c.car, "QUOTIENT"); for (args = args->u.c.cdr; CONSP(args); args = args->u.c.cdr) { long d = fixval(args->u.c.car, "QUOTIENT"); if (d == 0) return lisp_error("DIVISION BY ZERO", NIL); s /= d; } return mkfix(s);
}
static L f_remainder(L a, L b) { long y = fixval(b, "REMAINDER"); if (sig) return NIL; if (y == 0) return lisp_error("DIVISION BY ZERO", NIL); return mkfix(fixval(a, "REMAINDER") % y); }
static L f_add1(L x) { if (FLOP(x)) return mkflo(x->u.flo + 1); return mkfix(fixval(x, "ADD1") + 1); }
static L f_sub1(L x) { if (FLOP(x)) return mkflo(x->u.flo - 1); return mkfix(fixval(x, "SUB1") - 1); }
static L f_minus(L x) { if (FLOP(x)) return mkflo(-x->u.flo); return mkfix(-fixval(x, "MINUS")); }
static L f_abs(L x) { if (FLOP(x)) return mkflo(fabs(x->u.flo)); long v = fixval(x, "ABS"); return mkfix(v < 0 ? -v : v); }
static L f_dif2(L a, L b) { return mkfix(fixval(a, "*DIF") - fixval(b, "*DIF")); }
static L f_plus2(L a, L b) { return mkfix(fixval(a, "*PLUS") + fixval(b, "*PLUS")); }
static L f_times2(L a, L b) { return mkfix(fixval(a, "*TIMES") * fixval(b, "*TIMES")); }
static L f_quo2(L a, L b) { long y = fixval(b, "*QUO"); if (y == 0) return lisp_error("DIVISION BY ZERO", NIL); return mkfix(fixval(a, "*QUO") / y); }
static L f_greaterp(L args) { for (; CONSP(args) && CONSP(args->u.c.cdr); args = args->u.c.cdr) { if (!NUMP(args->u.c.car) || !NUMP(args->u.c.cdr->u.c.car)) return lisp_error("GREATERP NON-NUMERIC ARG", args); if (!(floval(args->u.c.car) > floval(args->u.c.cdr->u.c.car))) return NIL; } return T_; }
static L f_lessp(L args) { for (; CONSP(args) && CONSP(args->u.c.cdr); args = args->u.c.cdr) { if (!NUMP(args->u.c.car) || !NUMP(args->u.c.cdr->u.c.car)) return lisp_error("LESSP NON-NUMERIC ARG", args); if (!(floval(args->u.c.car) < floval(args->u.c.cdr->u.c.car))) return NIL; } return T_; }
static L f_great2(L a, L b) { return B(floval(a) > floval(b)); }
static L f_less2(L a, L b) { return B(floval(a) < floval(b)); }
static L f_zerop(L x) { return B(NUMP(x) && floval(x) == 0); }
static L f_minusp(L x) { return B(NUMP(x) && floval(x) < 0); }
static L f_plusp(L x) { return B(NUMP(x) && floval(x) > 0); }
static L f_max(L args) { if (!CONSP(args)) return NIL; L m = args->u.c.car; for (args = args->u.c.cdr; CONSP(args); args = args->u.c.cdr) if (floval(args->u.c.car) > floval(m)) m = args->u.c.car; return m; }
static L f_min(L args) { if (!CONSP(args)) return NIL; L m = args->u.c.car; for (args = args->u.c.cdr; CONSP(args); args = args->u.c.cdr) if (floval(args->u.c.car) < floval(m)) m = args->u.c.car; return m; }
static L f_fix(L x) { return mkfix((long)floor(floval(x))); }
static L f_float(L x) { return mkflo(floval(x)); }
/* F10 - SUBR RANDOM da imagem MOBY de 1971, opt-in.
 * DESASSEMBLAGEM COMPROVADA no MOBY: 007162..007175.
 * 007163 MOVE 2,000727       007164 MOVE 1,000726
 * 007165 MOVEM 1,000727      007166 LSHC 1,777735 (-35 decimal)
 * 007167 XORB 2,000726      007170 MOVE 1,000002
 * 007171 JRST 007054        007173 MOVE 1,016012
 * 007174 BLT 1,000727       007175 JRST 011673
 * As duas constantes gravadas na imagem em 007176 e 007177 também
 * estavam carregadas em 000726 e 000727 na captura MOBY; NÃO são
 * instruções, e constituem o estado de partida do dump.
 * Não emula o estado do ITS fora do dump ou o restante do PDP-10. */
#define F10_MASK36 0xfffffffffULL
#define F10_MASK35 0x7ffffffffULL
static unsigned long long f10_726=0267762113337ULL;
static unsigned long long f10_727=0155256071112ULL;
static void f10_reset(void){
    f10_726=0267762113337ULL; f10_727=0155256071112ULL;
}
static long f10_next(void){
    unsigned long long a1=f10_726, a2=f10_727, shifted, newword;
    f10_727=a1;
    /* PDP-10 LSHC 1,-35: palavra dupla AC1,AC2 deslocada à direita */
    shifted=((a1&F10_MASK35)<<1)|(a2>>35);
    newword=(a1^shifted)&F10_MASK36;
    f10_726=newword;
    return (newword&(1ULL<<35))?(long)(newword-(1ULL<<36)):(long)newword;
}
/* F13: RANDOM do MOBY 1971: sem ativação por variáveis do ambiente.
 * RANDOM sem argumentos -> próximo word da sequência; com 1 argumento ->
 * reinitialize state (a rota PDP-10 007173..007175), resultado NIL.
 * Compatível com palavras 36 bits em ambiente moderno de 64 bits.
 */
static L f_random(L args){
    if(CONSP(args)){
        if(CONSP(args->u.c.cdr))return lisp_error("RANDOM: invalid arity",args);
        f10_reset();
        return NIL;
    }
    return mkfix(f10_next());
}
/* BOOLE, MacLisp (manual MIT de 17/12/1975, sec. 8.8, p. 2-75):
 * op = 4 bits da tabela 00 01 10 11 (8,4,2,1).
 * Aceita >= 3 argumentos e reduz da esquerda para a direita.
 * Mantém sinal de fixnum PDP-10 (palavra de 36 bits).
 * Provisório no aspecto histórico: requer confronto com MacLisp 1971 em ITS.
 */
static unsigned long long f13_boole2(unsigned long k, unsigned long long x, unsigned long long y){
    unsigned long long u = 0;
    if(k & 8UL) u |= (~x & ~y);
    if(k & 4UL) u |= (~x &  y);
    if(k & 2UL) u |= ( x & ~y);
    if(k & 1UL) u |= ( x &  y);
    return u & F10_MASK36;
}
static L f_boole(L args){
    if(length(args)<3) return lisp_error("BOOLE TOO FEW ARGUMENTS", args);
    long k = fixval(car(args),"BOOLE");
    if(k<0 || k>15) return lisp_error("BOOLE INVALID OPERATION",args);
    L rest = cdr(args);
    unsigned long long u = (unsigned long long)fixval(car(rest),"BOOLE") & F10_MASK36;
    for(rest=cdr(rest); CONSP(rest); rest=cdr(rest)){
        unsigned long long v=(unsigned long long)fixval(car(rest),"BOOLE") & F10_MASK36;
        u=f13_boole2((unsigned long)k,u,v);
    }
    return mkfix((u & (1ULL<<35)) ? (long)(u-(1ULL<<36)) : (long)u);
}

/* F19: LSH opera sobre uma palavra PDP-10 de 36 bits, com preenchimento
 * por zero nos deslocamentos logicos. Deslocar inteiros C com sinal negativo
 * a esquerda, ou por n >= a largura do tipo, era comportamento indefinido.
 * Referencia: MacLisp Reference Manual MIT (08/04/1974), secao 7.8;
 * |n| > 36 e indefinido no manual: aqui explicitamos erro seguro (politica C99).
 * Confirmacao diferencial da versao MOBY/ITS 1971 continua pendente. */
static L f_lsh(L a, L b) {
    if (!FIXP(a)) return lisp_error("LSH FIRST ARGUMENT MUST BE FIXNUM", a);
    if (!FIXP(b)) return lisp_error("LSH SECOND ARGUMENT MUST BE FIXNUM", b);
    long x = a->u.fix, n = b->u.fix;
    if (n < -36 || n > 36) return lisp_error("LSH SHIFT COUNT OUT OF RANGE", b);
    unsigned long long u = (unsigned long long)x & F10_MASK36;
    if (n >= 0) u = (u << (unsigned)n) & F10_MASK36;
    else        u >>= (unsigned)(-n);
    return mkfix((u & (1ULL << 35)) ? (long)(u - (1ULL << 36)) : (long)u);
}

/* ── símbolos e propriedades ── */
static L pname_words(L sym) {
    /* PNAME como no PDP-10: 5 caracteres ASCII de 7 bits por palavra, da esquerda, bit 0 livre; fixnum de 36 bits com sinal */
    const char *s = symname(sym); L r = NIL, t = NULL; size_t n = strlen(s);
    for (size_t i = 0; i < n || i == 0; i += 5) {
        unsigned long w = 0;
        for (int k = 0; k < 5; k++) { unsigned long c = (i + k < n) ? ((unsigned char)s[i + k] & 0177) : 0; w |= c << (29 - 7 * k); }
        long v = (w & (1UL << 35)) ? (long)w - (1L << 36) : (long)w;
        L cell = list1(mkfix(v)); if (t) t->u.c.cdr = cell; else r = cell; t = cell;
        if (n == 0) break;
    }
    return r;
}
static L f_get(L x, L ind) {
    L result = NIL;
    if (SYMP(x)) {
        L c = getprop_cell(x, ind);
        if (c) result = c->u.c.car;
        else if (ind == S_PNAME) result = pname_words(x);
    } else if (CONSP(x)) {
        for (L p = x->u.c.cdr; CONSP(p) && CONSP(p->u.c.cdr); p = p->u.c.cdr->u.c.cdr)
            if (p->u.c.car == ind) { result = p->u.c.cdr->u.c.car; break; }
    }
    /* F22: a funcao WHO pesquisa a propriedade de uma LISTA de assercao.
       Registrar somente padroes alvo em canal opt-in, sem modificar result. */
    if (who_trace_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f22_record("GET_WHO_PATTERN", x, ind, result);
    if (f23_db_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f23_event("GET_WHO",x,result);
    if (f24_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f24_event("GET_WHO",x,result);
    if (f25_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f25_trace("GET_WHO",ind,x,result);
    return result;
}
static L f_getl(L x, L inds) {
    L p = SYMP(x) ? x->u.s.plist : CONSP(x) ? x->u.c.cdr : NIL;
    for (; CONSP(p) && CONSP(p->u.c.cdr); p = p->u.c.cdr->u.c.cdr) if (f_memq(p->u.c.car, inds) != NIL) return p;
    return NIL;
}
static L f_putprop(L x, L v, L ind) {
    if (who_trace_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f22_record("PUT_WHO_PATTERN",x,ind,v);
    if (f23_db_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f23_event("PUT_WHO",x,v);
    if (f24_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f24_event("PUT_WHO_BEFORE",x,v);
    if (SYMP(x)) { putprop(x, v, ind); return v; }
    if (CONSP(x)) {   /* plist desencarnada: (x ind val ...) */
        /* Propriedades em plist desencarnada seguem a mesma ordem de PUTPROP:
           a entrada redefinida torna-se a primeira. Preservar o objeto raiz. */
        L prev=x;
        for (L p=x->u.c.cdr; CONSP(p) && CONSP(p->u.c.cdr); p=p->u.c.cdr->u.c.cdr) {
            if (p->u.c.car == ind) {
                prev->u.c.cdr=p->u.c.cdr->u.c.cdr;
                break;
            }
            prev=p->u.c.cdr;
        }
        x->u.c.cdr=cons(ind,cons(v,x->u.c.cdr));
        return v;
    }
    return lisp_error("PUTPROP WRONG TYPE ARG", x);
}
static L f_remprop(L x, L ind) {
    if (who_trace_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f22_record("REMOVE_WHO_PATTERN",x,ind,NULL);
    if (f23_db_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f23_event("REMOVE_WHO",x,ind);
    if (f24_fp && f22_named(ind,"WHO") && f22_watch_pattern(x))
        f24_event("REMOVE_WHO_BEFORE",x,ind);
    if (SYMP(x)) return remprop(x, ind);
    if (CONSP(x)) { L prev = x; for (L p = x->u.c.cdr; CONSP(p) && CONSP(p->u.c.cdr); prev = p->u.c.cdr, p = p->u.c.cdr->u.c.cdr) if (p->u.c.car == ind) { prev->u.c.cdr = p->u.c.cdr->u.c.cdr; return T_; } }
    return NIL;
}
static L f_plist(L x) { return SYMP(x) ? x->u.s.plist : NIL; }
static L f_setplist(L x, L p) { if (SYMP(x)) { x->u.s.plist = p; x->u.s.valcell = getprop_cell(x, S_VALUE); } return p; }
static L f_set(L s, L v) { if (!SYMP(s) || s == NIL || s == T_) return lisp_error("SET WRONG TYPE ARG", s); setval(s, v); return v; }
/* MacLisp Moon (1978), secao GENSYM: argumentos opcionais alteram o
 * prefixo de um caractere ou definem o contador; a chamada devolve um
 * simbolo NAO internado. O comportamento de 1971 ainda requer prova no ITS. */
static L f_gensym(L args) {
    if (CONSP(args)) {
        L x = args->u.c.car;
        if (CONSP(args->u.c.cdr)) return lisp_error("GENSYM TOO MANY ARGUMENTS", args);
        if (FIXP(x)) {
            gensym_counter->u.fix = x->u.fix;
            /* Com argumento numerico o numero passado eh usado nesta chamada. */
        } else if (SYMP(x) && symname(x)[0]) {
            gensym_prefix = symname(x)[0];
            gensym_counter->u.fix++;
        } else return lisp_error("GENSYM WRONG TYPE ARG", x);
    } else gensym_counter->u.fix++;
    char b[80];
    snprintf(b, sizeof b, "%c%04ld", gensym_prefix, gensym_counter->u.fix);
    return make_uninterned_symbol(b);
}
static L chars_to_string(L l, char *buf, int cap) {
    int n = 0;
    for (; CONSP(l) && n < cap - 1; l = l->u.c.cdr) {
        L c = l->u.c.car;
        if (SYMP(c)) { const char *s = symname(c); while (*s && n < cap - 1) buf[n++] = *s++; }
        else if (FIXP(c)) { if (c->u.fix >= 0 && c->u.fix <= 9) buf[n++] = (char)('0' + c->u.fix); else if (c->u.fix < 128 && c->u.fix >= 0) buf[n++] = (char)c->u.fix; else { char d[32]; snprintf(d, sizeof d, "%lo", c->u.fix); for (char *p = d; *p && n < cap - 1; p++) buf[n++] = *p; } }
    }
    buf[n] = 0; return NIL;
}
static L f_maknam(L l) { char b[512]; chars_to_string(l, b, sizeof b); return make_uninterned_symbol(b); }
static L f_readlist(L l) {
    char b[512]; chars_to_string(l, b, sizeof b);
    Stream s = { b, 0, strlen(b), 0, "READLIST", 1, 0 }; Stream *save = in; in = &s;
    L x = read_obj(); in = save;
    if (x == EOF_OBJ) return NIL;
    return x;
}
/* Capture PRIN1/PRINC in an ISO C99 tmpfile (no GNU open_memstream). */
static char *capture_obj(L x, int slashify, size_t *size) {
    FILE *f = tmpfile();
    if (!f) { fprintf(stderr, "tmpfile indisponivel\n"); exit(2); }
    FILE *saved = out; int saved_col = column;
    out = f; print_obj(x, slashify);
    if (fflush(f) != 0 || fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "falha ao gravar tmpfile\n"); exit(2);
    }
    long n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fprintf(stderr, "falha ao ler tmpfile\n"); exit(2);
    }
    char *b = malloc((size_t)n + 1);
    if (!b) { fprintf(stderr, "sem memoria\n"); exit(2); }
    if (fread(b, 1, (size_t)n, f) != (size_t)n) {
        fprintf(stderr, "falha ao ler tmpfile\n"); exit(2);
    }
    b[n] = 0;
    fclose(f); out = saved; column = saved_col;
    if (size) *size = (size_t)n;
    return b;
}
static L explode(L x, int slashify) {
    size_t len;
    char *buf = capture_obj(x, slashify, &len);
    L r = NIL;
    for (size_t i = len; i > 0; i--) {
        char c[2] = { buf[i-1], 0 };
        r = cons(isdigit((unsigned char)c[0]) && !slashify ? mkfix(c[0] - '0') : intern(c), r);
    }
    free(buf); return r;
}
static L f_explode(L x) { return explode(x, 1); }
static L f_explodec(L x) {
    size_t len;
    char *buf = capture_obj(x, 0, &len);
    L r = NIL;
    for (size_t i = len; i > 0; i--) {
        char c[2] = { buf[i-1], 0 };
        r = cons(intern(c), r);
    }
    free(buf); return r;
}
static L f_flatsize(L x) {
    size_t len; char *buf = capture_obj(x, 1, &len);
    free(buf); return mkfix((long)len);
}
static L f_intern(L x) { return SYMP(x) ? intern(symname(x)) : x; }
static L f_samepnamep(L a, L b) { return B(SYMP(a) && SYMP(b) && !strcmp(symname(a), symname(b))); }
static L f_ascii(L n) { char b[2] = { (char)fixval(n, "ASCII"), 0 }; return intern(b); }

/* ── controle ── */
static L f_quote(L args) { return car(args); }
static L f_cond(L args) {
    for (; CONSP(args); args = args->u.c.cdr) {
        L clause = args->u.c.car; if (!CONSP(clause)) continue;
        L v = eval(clause->u.c.car); if (sig) return NIL;
        if (v != NIL) { if (!CONSP(clause->u.c.cdr)) return v; return progn(clause->u.c.cdr); }
    }
    return NIL;
}
static L f_and(L args) { L v = T_; for (; CONSP(args); args = args->u.c.cdr) { v = eval(args->u.c.car); if (sig || v == NIL) return NIL; } return v; }
static L f_or(L args) { for (; CONSP(args); args = args->u.c.cdr) { L v = eval(args->u.c.car); if (sig) return NIL; if (v != NIL) return v; } return NIL; }
static L f_progn(L args) { return args == NIL ? NIL : f_last(args)->u.c.car; }     /* SUBR: argumentos já avaliados */
static L f_prog2(L args) { return car(cdr(args)); }
static L f_setq(L args) {
    L v = NIL;
    for (; CONSP(args) && CONSP(args->u.c.cdr); args = args->u.c.cdr->u.c.cdr) {
        L s = args->u.c.car; v = eval(args->u.c.cdr->u.c.car); if (sig) return NIL;
        if (!SYMP(s) || s == NIL) return lisp_error("SETQ WRONG TYPE ARG", s);
        setval(s, v);
    }
    return v;
}
static L f_prog(L args) {
    L vars = car(args), body = cdr(args); int mark = bsp;
    for (L v = vars; CONSP(v); v = v->u.c.cdr) { L s = v->u.c.car; if (CONSP(s)) { L init = eval(car(cdr(s))); if (sig) { unbind_to(mark); return NIL; } bind(s->u.c.car, init); } else bind(s, NIL); }
    L pc = body, result = NIL;
    while (CONSP(pc)) {
        L stmt = pc->u.c.car;
        if (!CONSP(stmt)) { pc = pc->u.c.cdr; continue; }          /* rótulo */
        eval(stmt);
        if (sig == SIG_GO) {
            L p; for (p = body; CONSP(p); p = p->u.c.cdr) if (!CONSP(p->u.c.car) && eq(p->u.c.car, sig_tag)) break;
            if (CONSP(p)) { sig = SIG_NONE; pc = p->u.c.cdr; continue; }
            break;                                                 /* rótulo de um PROG externo */
        }
        if (sig == SIG_RETURN) { sig = SIG_NONE; result = sig_val; break; }
        if (sig) break;
        pc = pc->u.c.cdr;
    }
    unbind_to(mark);
    return result;
}
static L f_go(L args) { if (sig) return NIL; L tag = car(args); if (CONSP(tag)) { tag = eval(tag); if (sig) return NIL; } sig = SIG_GO; sig_tag = tag; return NIL; }
static L f_return(L args) { L v = eval(car(args)); if (sig) return NIL; sig = SIG_RETURN; sig_val = v; return NIL; }
static L f_errset(L args) {
    L form = car(args); int flag = CONSP(cdr(args)) ? 0 : 1;      /* (ERRSET x) imprime erros; (ERRSET x NIL) não */
    if (CONSP(cdr(args))) { L f = eval(car(cdr(args))); if (sig) return NIL; flag = f != NIL; }
    int mark = bsp, save_silent = errset_silent; errset_depth++; if (!flag) errset_silent++;
    L v = eval(form);
    errset_depth--; errset_silent = save_silent;
    if (sig == SIG_ERR) {
        if (!err_quiet && (flag || getenv("ERRSHOW"))) { terpri(); outs(";"); outs(err_msg); outs(" - "); prin1(err_obj); terpri(); }
        L r = err_quiet ? sig_val : NIL; sig = SIG_NONE; unbind_to(mark);
        return r == NIL ? NIL : r;
    }
    if (sig) { unbind_to(mark); return NIL; }
    return list1(v);
}
static L f_err(L args) { if (sig) return NIL; sig = SIG_ERR; err_quiet = 1; sig_val = CONSP(args) ? args->u.c.car : NIL; err_obj = NIL; strcpy(err_msg, "ERR"); return NIL; }
static L f_error(L args) {
    L msg = car(args), obj = car(cdr(args));
    char *buf = capture_obj(msg, 0, NULL);
    L r = lisp_error(buf, obj);
    free(buf); return r;
}
static L f_eval(L x) { return eval(x); }
static L f_apply(L fn, L args) { return apply(fn, args); }
static L f_function(L args) { return car(args); }
static L f_comment(L args) { (void)args; return intern("COMMENT"); }
static L f_declare(L args) { (void)args; return NIL; }
static L f_defun(L args) {
    L name = car(args), rest = cdr(args), kind = S_EXPR;
    if ((name == S_FEXPR || name == S_MACRO || name == S_EXPR) && SYMP(car(rest))) { kind = name; name = car(rest); rest = cdr(rest); }   /* (DEFUN FEXPR NOME ...) */
    L second = car(rest);
    if (second == S_FEXPR || second == S_MACRO || second == S_EXPR) { kind = second; rest = cdr(rest); }
    L lam = cons(S_LAMBDA, rest);                /* (LAMBDA params . body) */
    putprop(name, lam, kind);
    return name;
}
static L f_defprop(L args) { L s = car(args), v = car(cdr(args)), ind = car(cdr(cdr(args))); f_putprop(s, v, ind); return s; }
static L f_deflist(L args) { L ind = car(args); for (L p = cdr(args); CONSP(p); p = p->u.c.cdr) { L e = p->u.c.car; f_putprop(car(e), car(cdr(e)), ind); } return ind; }
static L mapfn(L args, int kind) {
    /* F18: o numero de listas deve determinar o tamanho do vetor de cursores.
     * Na F17 cur[8] era lido ate nl, mesmo para nl > 8, causando
     * stack-buffer-overflow no MAP/MAPC/MAPCAR/MAPLIST/MAPCAN/MAPCON.
     * malloc/free sao detalhes do runtime C; nao afetam as listas Lisp. */
    L fn = car(args), lists = cdr(args);
    int nl = length(lists);
    if (nl == 0) return NIL;
    if ((size_t)nl > (size_t)-1 / sizeof(L))
        return lisp_error("MAP TOO MANY LISTS", args);
    L *cur = malloc((size_t)nl * sizeof *cur);
    if (!cur) return lisp_error("MAP ALLOCATION FAILED", args);
    int i = 0;
    for (L p = lists; CONSP(p); p = p->u.c.cdr) cur[i++] = p->u.c.car;
    L res = NIL, tail = NULL, first = cur[0];
    for (;;) {
        for (int k = 0; k < nl; k++) if (!CONSP(cur[k])) goto done;
        L a = NIL, at = NULL;
        for (int k = 0; k < nl; k++) {
            L cell = list1(kind == 2 || kind == 4 || kind == 5 ? cur[k] : cur[k]->u.c.car);
            if (at) at->u.c.cdr = cell; else a = cell;
            at = cell;
        }
        L v = apply(fn, a);
        if (sig) { free(cur); return NIL; }
        if (kind == 1 || kind == 2) {
            L cell = list1(v);
            if (tail) tail->u.c.cdr = cell; else res = cell;
            tail = cell;
        } else if (kind == 3 || kind == 4) {
            if (CONSP(v)) {
                if (tail) tail->u.c.cdr = v; else res = v;
                tail = v;
                while (CONSP(tail->u.c.cdr)) tail = tail->u.c.cdr;
            }
        }
        for (int k = 0; k < nl; k++) cur[k] = cur[k]->u.c.cdr;
    }
done:
    free(cur);
    if (kind == 0 || kind == 5) return first;
    return res;
}
static L f_mapc(L a) { return mapfn(a, 0); }
static L f_mapcar(L a) { return mapfn(a, 1); }
static L f_maplist(L a) { return mapfn(a, 2); }
static L f_mapcan(L a) { return mapfn(a, 3); }
static L f_mapcon(L a) { return mapfn(a, 4); }
static L f_map(L a) { return mapfn(a, 5); }
/* F17: ARG/SETARG/LISTIFY devem operar somente no quadro LEXPR ativo.
 * Conforme Moon 1978, ARG e SETARG validam 1 <= indice <= numero de
 * argumentos. LISTIFY positivo copia o prefixo; negativo copia o sufixo.
 * A comparacao especifica com MacLisp 1971 (MOBY) ainda esta pendente. */
static L lexpr_validate_index(L num, const char *name) {
    if (!FIXP(num)) {
        char msg[100]; snprintf(msg, sizeof msg, "%s INDEX NOT FIXNUM", name);
        return lisp_error(msg, num);
    }
    long k = num->u.fix;
    if (k < 1 || k > (long)lexpr_n[lexpr_sp - 1]) {
        char msg[100]; snprintf(msg, sizeof msg, "%s INDEX OUT OF RANGE", name);
        return lisp_error(msg, num);
    }
    return num;
}
static L f_arg(L args) {
    if (lexpr_sp == 0) return lisp_error("ARG NOT INSIDE LEXPR", NIL);
    L n = CONSP(args) ? args->u.c.car : NIL;
    if (n == NIL) return mkfix(lexpr_n[lexpr_sp - 1]);
    if (!lexpr_validate_index(n, "ARG") || sig) return NIL;
    L a = lexpr_args[lexpr_sp - 1];
    for (long i = 1; i < n->u.fix; i++) a = a->u.c.cdr;
    return a->u.c.car;
}
static L f_setarg(L n, L v) {
    if (lexpr_sp == 0) return lisp_error("SETARG NOT INSIDE LEXPR", NIL);
    if (!lexpr_validate_index(n, "SETARG") || sig) return NIL;
    L a = lexpr_args[lexpr_sp - 1];
    for (long i = 1; i < n->u.fix; i++) a = a->u.c.cdr;
    a->u.c.car = v;
    return v;
}
static L f_listify(L n) {
    if (lexpr_sp == 0) return lisp_error("LISTIFY NOT INSIDE LEXPR", NIL);
    if (!FIXP(n)) return lisp_error("LISTIFY COUNT NOT FIXNUM", n);
    long count = (long)lexpr_n[lexpr_sp - 1];
    long k = n->u.fix;
    /* k == LONG_MIN nao pode ser negado: comparar sem overflow primeiro. */
    if (k > count || k < -count) return lisp_error("LISTIFY COUNT OUT OF RANGE", n);
    long how_many = k >= 0 ? k : -k;
    L a = lexpr_args[lexpr_sp - 1];
    if (k < 0) for (long i = 0; i < count - how_many; i++) a = a->u.c.cdr;
    L r = NIL, t = NULL;
    for (long i = 0; i < how_many; i++, a = a->u.c.cdr) {
        L c = list1(a->u.c.car);
        if (t) t->u.c.cdr = c; else r = c;
        t = c;
    }
    return r;
}
static L f_do(L args) {
    /* DO antigo: (DO var init step teste . corpo); DO novo: (DO ((var init step)...) (teste . res) . corpo) */
    int mark = bsp;
    if (CONSP(car(args)) || car(args) == NIL) {
        L specs = car(args), test = car(cdr(args)), body = cdr(cdr(args));
        for (L s = specs; CONSP(s); s = s->u.c.cdr) { L sp = s->u.c.car; L init = eval(car(cdr(sp))); if (sig) { unbind_to(mark); return NIL; } bind(car(sp), init); }
        for (;;) {
            L t = eval(car(test)); if (sig) break;
            if (t != NIL) { L r = progn(cdr(test)); unbind_to(mark); return r; }
            f_prog(cons(NIL, body)); if (sig) break;
            L vals = NIL; for (L s = specs; CONSP(s); s = s->u.c.cdr) { L sp = s->u.c.car; L step = cdr(cdr(sp)) != NIL ? eval(car(cdr(cdr(sp)))) : symval(car(sp)); if (sig) break; vals = cons(step, vals); }
            if (sig) break;
            vals = nreverse(vals);
            for (L s = specs; CONSP(s); s = s->u.c.cdr, vals = vals->u.c.cdr) setval(car(s->u.c.car), vals->u.c.car);
        }
        unbind_to(mark); return NIL;
    }
    L var = car(args); L init = eval(car(cdr(args))); if (sig) return NIL;
    bind(var, init);
    L step = car(cdr(cdr(args))), test = car(cdr(cdr(cdr(args)))), body = cdr(cdr(cdr(cdr(args))));
    for (;;) {
        L t = eval(test); if (sig || t != NIL) break;
        f_prog(cons(NIL, body)); if (sig) break;
        L nv = eval(step); if (sig) break; setval(var, nv);
    }
    unbind_to(mark); return NIL;
}
static L f_catch(L args) { L form = car(args); L v = eval(form); if (sig == SIG_THROW) { sig = SIG_NONE; return sig_val; } return v; }
static L f_throw(L v) { if (sig) return NIL; sig = SIG_THROW; sig_val = v; return NIL; }

/* ── entrada e saída ── */
static L f_print(L x) { terpri(); prin1(x); outc(' '); return x; }
static L f_prin1(L x) { prin1(x); return x; }
static L f_princ(L x) { princ(x); return x; }
static L f_terpri(void) { terpri(); return NIL; }
static L f_tyo(L n) { long c = fixval(n, "TYO"); if (c == 13) return n; if (c == 14 || c == 12) { terpri(); return n; } outc((int)c); return n; }
static L f_read(L args) { (void)args; L x = read_obj(); if (x == EOF_OBJ) { if (CONSP(args)) return args->u.c.car; if (in == tty) { fprintf(stderr, "fim da entrada\n"); exit(3); } return lisp_error("END OF FILE", NIL); } return x; }
static L f_readch(L args) {
    int c = tyi();
    if (c < 0) { if (CONSP(args)) return args->u.c.car; if (in == tty) { fprintf(stderr, "fim da entrada\n"); exit(3); } return lisp_error("END OF FILE", NIL); }
    if (c == '\r') { c = tyipeek() == '\n' ? (tyi(), '\r') : '\r'; }
    if (isdigit(c)) return mkfix(c - '0');                /* z.syscom espera (NUMBERP CHAR) para dígitos */
    char b[2] = { (char)c, 0 }; return intern(b);
}
static L f_tyi(L args) { int c = tyi(); if (c < 0) { if (CONSP(args)) return args->u.c.car; return lisp_error("END OF FILE", NIL); } return mkfix(c); }
static L f_tyipeek(void) { int c = tyipeek(); return mkfix(c < 0 ? -1 : c); }
static L f_ioc(L args) { (void)args; return NIL; }
static L f_nilfn(L args) { (void)args; return NIL; }

static L f_status(L args) {
    L what = car(args);
    const char *w = SYMP(what) ? symname(what) : "";
    if (!strncmp(w, "UREAD", 5)) return in && in->name ? list2(intern(in->name), mkfix(0)) : NIL;
    if (!strncmp(w, "TTY", 3)) return NIL;
    if (!strncmp(w, "MACRO", 5)) { L c = car(cdr(args)); int ch = SYMP(c) ? (unsigned char)symname(c)[0] : (int)fixval(c, "STATUS"); return macro_table[ch] ? macro_table[ch] : NIL; }
    if (!strncmp(w, "CHARPOS", 7) || !strncmp(w, "LINEL", 5)) return mkfix(column);
    if (!strncmp(w, "DATE", 4)) { time_t t = time(NULL); struct tm *tm = localtime(&t); return list3(mkfix(tm->tm_year % 100), mkfix(tm->tm_mon + 1), mkfix(tm->tm_mday)); }
    return NIL;
}
static L f_sstatus(L args) {
    L what = car(args);
    const char *w = SYMP(what) ? symname(what) : "";
    if (!strncmp(w, "MACRO", 5)) {
        L c = car(cdr(args)); L fn = eval(car(cdr(cdr(args)))); if (sig) return NIL;
        int ch = SYMP(c) ? (unsigned char)symname(c)[0] : (int)fixval(c, "SSTATUS");
        if (ch == 0) return NIL;
        macro_table[ch] = fn == NIL ? NULL : fn;
        macro_splice[ch] = car(cdr(cdr(cdr(args)))) != NIL;
        return fn;
    }
    return NIL;
}
static L f_time(void) { return mkfix((long)time(NULL)); }
static L f_runtime(void) { return mkfix((long)(clock() * 1000000L / CLOCKS_PER_SEC)); }

/* ───────────────────────── carga de arquivos ───────────────────────── */
static int verbose_load = 0;
static int load_file(const char *path, const char *label) {
    size_t len; char *buf = read_file(path, &len);
    if (!buf) { fprintf(stderr, "nao abri %s\n", path); return -1; }
    Stream s = { buf, 0, len, 0, label ? label : path, 1, 0 };
    Stream *save = in; in = &s;
    int nforms = 0, nerr = 0;
    for (;;) {
        int start_line = s.line;
        sig = SIG_NONE; depth = 0; bsp = 0; lexpr_sp = 0;
        L x = read_obj();
        if (sig) { fprintf(stderr, "%s:%d: erro de leitura: %s\n", path, start_line, err_msg); nerr++; sig = SIG_NONE; break; }
        if (x == EOF_OBJ) break;
        if (CONSP(x) && SYMP(x->u.c.car) && !strcmp(symname(x->u.c.car), "LAP")) {    /* código de máquina: pula até o fim do bloco */
            continue;
        }
        nforms++;
        L v = eval(x);
        if (sig) {
            if (sig == SIG_ERR) { fprintf(stderr, "%s:%d: %s", path, start_line, err_msg); if (err_obj && err_obj != NIL) { fputs(" - ", stderr); FILE *so = out; out = stderr; prin1(err_obj); out = so; } fputc('\n', stderr); }
            else fprintf(stderr, "%s:%d: sinal %d fora de lugar\n", path, start_line, sig);
            nerr++; sig = SIG_NONE;
        } else if (verbose_load && CONSP(x)) { FILE *so = out; out = stderr; prin1(v); outc('\n'); out = so; }
    }
    in = save; free(buf);
    fprintf(stderr, "%-12s %4d formas, %d erros\n", label ? label : path, nforms, nerr);
    return nerr;
}
static L f_load(L x) { char path[512]; snprintf(path, sizeof path, "%s", SYMP(x) ? symname(x) : ""); for (char *p = path; *p; p++) *p = tolower(*p); load_file(path, NULL); return T_; }

/* stubs das funções de display e do braço (z.mover é código de máquina do PDP-10) */
static L f_stub(L args) { (void)args; return T_; }

/* ───────────────────────── inicialização ───────────────────────── */
static void init(void) {
    NIL = mk(T_SYM); NIL->u.s.name = "NIL"; NIL->u.s.plist = NIL; NIL->u.s.valcell = NULL;
    { unsigned h = hash("NIL"); struct hent *e = malloc(sizeof *e); e->name = "NIL"; e->sym = NIL; e->next = htab[h]; htab[h] = e; }
    T_ = intern("T"); UNBOUND = intern("*UNBOUND*"); EOF_OBJ = intern("*EOF*");
    DOT_MARK = mk(T_SYM); DOT_MARK->u.s.name = "."; DOT_MARK->u.s.plist = NIL; DOT_MARK->u.s.valcell = NULL;
    S_QUOTE = intern("QUOTE"); S_LAMBDA = intern("LAMBDA"); S_EXPR = intern("EXPR"); S_FEXPR = intern("FEXPR"); S_SUBR = intern("SUBR");
    S_FSUBR = intern("FSUBR"); S_LSUBR = intern("LSUBR"); S_MACRO = intern("MACRO"); S_VALUE = intern("VALUE"); S_FUNCTION = intern("FUNCTION");
    S_DOT = intern("."); S_COND = intern("COND"); S_PROG = intern("PROG"); S_GO = intern("GO"); S_RETURN = intern("RETURN"); S_SETQ = intern("SETQ");
    S_AND = intern("AND"); S_OR = intern("OR"); S_ERRSET = intern("ERRSET"); S_ERR = intern("ERR"); S_IBASE = intern("IBASE"); S_BASE = intern("BASE");
    S_NOPOINT = intern("*NOPOINT"); S_PNAME = intern("PNAME"); S_READ = intern("READ"); S_PRIN1 = intern("PRIN1"); S_T = T_;
    putprop(NIL, NIL, S_VALUE);            /* (CADR NIL) = VALUE, como no Lisp da imagem */
    setval(T_, T_);
    setval(S_IBASE, mkfix(8)); setval(S_BASE, mkfix(8)); setval(S_NOPOINT, T_);
    gensym_counter = mkfix(0);
    out = stdout;
    S_LINEL = intern("LINEL"); setval(S_LINEL, mkfix(72)); S_CHRCT = intern("CHRCT"); setval(S_CHRCT, mkfix(72));

    /* listas */
    defsubr1("CAR", f_car, 1, 1); defsubr1("CDR", f_cdr, 1, 1);
    defsubr1("CAAR", f_caar, 1, 1); defsubr1("CADR", f_cadr, 1, 1); defsubr1("CDAR", f_cdar, 1, 1); defsubr1("CDDR", f_cddr, 1, 1);
    defsubr1("CAAAR", f_caaar, 1, 1); defsubr1("CAADR", f_caadr, 1, 1); defsubr1("CADAR", f_cadar, 1, 1); defsubr1("CADDR", f_caddr, 1, 1);
    defsubr1("CDAAR", f_cdaar, 1, 1); defsubr1("CDADR", f_cdadr, 1, 1); defsubr1("CDDAR", f_cddar, 1, 1); defsubr1("CDDDR", f_cdddr, 1, 1);
    defsubr1("CAAAAR", f_caaaar, 1, 1); defsubr1("CAAADR", f_caaadr, 1, 1); defsubr1("CAADAR", f_caadar, 1, 1); defsubr1("CAADDR", f_caaddr, 1, 1);
    defsubr1("CADAAR", f_cadaar, 1, 1); defsubr1("CADADR", f_cadadr, 1, 1); defsubr1("CADDAR", f_caddar, 1, 1); defsubr1("CADDDR", f_cadddr, 1, 1);
    defsubr1("CDAAAR", f_cdaaar, 1, 1); defsubr1("CDAADR", f_cdaadr, 1, 1); defsubr1("CDADAR", f_cdadar, 1, 1); defsubr1("CDADDR", f_cdaddr, 1, 1);
    defsubr1("CDDAAR", f_cddaar, 1, 1); defsubr1("CDDADR", f_cddadr, 1, 1); defsubr1("CDDDAR", f_cdddar, 1, 1); defsubr1("CDDDDR", f_cddddr, 1, 1);
    defsubr2("CONS", f_cons, 2, 2); defsubr1("NCONS", f_ncons, 1, 1); defsubr2("XCONS", f_xcons, 2, 2);
    deflsubr("LIST", f_list); deflsubr("APPEND", f_append); deflsubr("NCONC", f_nconc);
    defsubr1("REVERSE", f_reverse, 1, 1); defsubr1("NREVERSE", f_nreverse, 1, 1); defsubr1("LENGTH", f_length, 1, 1); defsubr1("LAST", f_last, 1, 1);
    defsubr2("MEMQ", f_memq, 2, 2); defsubr2("MEMBER", f_member, 2, 2); defsubr2("ASSQ", f_assq, 2, 2); defsubr2("ASSOC", f_assoc, 2, 2);
    defsubr3("SASSQ", f_sassq, 3, 3); defsubr3("SASSOC", f_sassoc, 3, 3); defsubr2("DELQ", f_delq, 2, 2); defsubr2("DELETE", f_delete, 2, 2);
    defsubr3("SUBST", f_subst, 3, 3); defsubr2("SUBLIS", f_sublis, 2, 2); defsubr2("RPLACA", f_rplaca, 2, 2); defsubr2("RPLACD", f_rplacd, 2, 2);
    /* predicados */
    defsubr1("ATOM", f_atom, 1, 1); defsubr1("NULL", f_null, 1, 1); defsubr1("NOT", f_null, 1, 1); defsubr2("EQ", f_eq, 2, 2); defsubr2("EQUAL", f_equal, 2, 2);
    defsubr1("NUMBERP", f_numberp, 1, 1); defsubr1("FIXP", f_fixp, 1, 1); defsubr1("FLOATP", f_floatp, 1, 1); defsubr1("BOUNDP", f_boundp, 1, 1);
    /* aritmética */
    deflsubr("PLUS", f_plus); deflsubr("+", f_plus); deflsubr("TIMES", f_times); deflsubr("*", f_times); deflsubr("DIFFERENCE", f_difference); deflsubr("-", f_difference);
    deflsubr("QUOTIENT", f_quotient); deflsubr("//", f_quotient); defsubr2("REMAINDER", f_remainder, 2, 2); defsubr2("\\", f_remainder, 2, 2);
    defsubr1("ADD1", f_add1, 1, 1); defsubr1("1+", f_add1, 1, 1); defsubr1("SUB1", f_sub1, 1, 1); defsubr1("1-", f_sub1, 1, 1); defsubr1("MINUS", f_minus, 1, 1); defsubr1("ABS", f_abs, 1, 1);
    defsubr2("*DIF", f_dif2, 2, 2); defsubr2("*PLUS", f_plus2, 2, 2); defsubr2("*TIMES", f_times2, 2, 2); defsubr2("*QUO", f_quo2, 2, 2);
    deflsubr("GREATERP", f_greaterp); deflsubr("LESSP", f_lessp); deflsubr(">", f_greaterp); deflsubr("<", f_lessp); defsubr2("*GREAT", f_great2, 2, 2); defsubr2("*LESS", f_less2, 2, 2);
    defsubr1("ZEROP", f_zerop, 1, 1); defsubr1("MINUSP", f_minusp, 1, 1); defsubr1("PLUSP", f_plusp, 1, 1); deflsubr("MAX", f_max); deflsubr("MIN", f_min);
    defsubr1("FIX", f_fix, 1, 1); defsubr1("FLOAT", f_float, 1, 1); deflsubr("RANDOM", f_random); deflsubr("BOOLE", f_boole); defsubr2("LSH", f_lsh, 2, 2);
    /* símbolos */
    defsubr2("GET", f_get, 2, 2); defsubr2("GETL", f_getl, 2, 2); defsubr3("PUTPROP", f_putprop, 3, 3); defsubr2("REMPROP", f_remprop, 2, 2);
    defsubr1("PLIST", f_plist, 1, 1); defsubr2("SETPLIST", f_setplist, 2, 2); defsubr2("SET", f_set, 2, 2); deflsubr("GENSYM", f_gensym);
    defsubr1("MAKNAM", f_maknam, 1, 1); defsubr1("READLIST", f_readlist, 1, 1); defsubr1("EXPLODE", f_explode, 1, 1); defsubr1("EXPLODEC", f_explodec, 1, 1);
    defsubr1("FLATSIZE", f_flatsize, 1, 1); defsubr1("INTERN", f_intern, 1, 1); defsubr2("SAMEPNAMEP", f_samepnamep, 2, 2); defsubr1("ASCII", f_ascii, 1, 1);
    /* controle */
    deffsubr("QUOTE", f_quote); deffsubr("Q", f_quote); deffsubr("FUNCTION", f_function); deffsubr("*FUNCTION", f_function); deffsubr("COND", f_cond); deffsubr("AND", f_and); deffsubr("OR", f_or);
    deflsubr("PROGN", f_progn); deflsubr("PROG2", f_prog2); deffsubr("SETQ", f_setq); deffsubr("PROG", f_prog); deffsubr("GO", f_go); deffsubr("RETURN", f_return);
    deffsubr("ERRSET", f_errset); deflsubr("ERR", f_err); deflsubr("ERROR", f_error); defsubr1("EVAL", f_eval, 1, 2); defsubr2("APPLY", f_apply, 2, 3);
    deffsubr("COMMENT", f_comment); deffsubr("DECLARE", f_declare); deffsubr("DEFUN", f_defun); deffsubr("DEFPROP", f_defprop); deffsubr("DEFLIST", f_deflist);
    deflsubr("MAPC", f_mapc); deflsubr("MAPCAR", f_mapcar); deflsubr("MAPLIST", f_maplist); deflsubr("MAPCAN", f_mapcan); deflsubr("MAPCON", f_mapcon); deflsubr("MAP", f_map);
    deflsubr("ARG", f_arg); defsubr2("SETARG", f_setarg, 2, 2); defsubr1("LISTIFY", f_listify, 1, 1); deffsubr("DO", f_do); deffsubr("CATCH", f_catch); defsubr1("THROW", f_throw, 1, 2);
    /* E/S */
    defsubr1("PRINT", f_print, 1, 2); defsubr1("PRIN1", f_prin1, 1, 2); defsubr1("PRINC", f_princ, 1, 2); defsubr0("TERPRI", f_terpri, 0, 1); defsubr1("TYO", f_tyo, 1, 2);
    deflsubr("READ", f_read); deflsubr("READCH", f_readch); deflsubr("TYI", f_tyi); defsubr0("TYIPEEK", f_tyipeek, 0, 1);
    deffsubr("IOC", f_ioc); deffsubr("STATUS", f_status); deffsubr("SSTATUS", f_sstatus); defsubr0("TIME", f_time, 0, 0); defsubr0("RUNTIME", f_runtime, 0, 0);
    deflsubr("UREAD", f_nilfn); deflsubr("UWRITE", f_nilfn); deflsubr("UFILE", f_nilfn); deflsubr("CRUNIT", f_nilfn); deflsubr("GC", f_nilfn); deflsubr("GCTWA", f_nilfn);
    deflsubr("ALLOC", f_nilfn); deflsubr("*RSET", f_nilfn); deflsubr("NOUUO", f_nilfn); deflsubr("VALRET", f_nilfn); deflsubr("SLEEP", f_nilfn); deflsubr("CURSORPOS", f_nilfn);
    deflsubr("REMOB", f_nilfn); deflsubr("MAKOBLIST", f_nilfn); deflsubr("BREAK", f_nilfn); deflsubr("PAGEBPORG", f_nilfn); deflsubr("SYMBOLS", f_nilfn);
    defsubr1("LOAD", f_load, 1, 1);
    /* braço e display: código de máquina em z.mover; aqui, sem efeito */
    const char *stubs[] = { "MOVETO", "GRASP", "UNGRASP", "DMOVR", "DMOVE", "DMOVR3", "DMOVE3", "DISARR", "DISP", "DLISTA", "DBRIGHT", "DCHK", "DDIST", "DGLIDE", "DMSK",
                            "HMOVE", "NUMC2", "NUMC3", "COORD", "TIMER", "BLINK", "DISME", "DISYOU", "DISAD", "DISINI", "UKILL", "NEXTPLOT", "PLOT", "PLOTLIST", "PLOTTEXT",
                            "READPIC", "WRITEPIC", "PICARRAY", "LPEN", "VIDI", "VIDLIN", "VIDLOG", "FIXX", "ADDBYT", "GETARR", NULL };
    for (int i = 0; stubs[i]; i++) deflsubr(stubs[i], f_stub);
}

/* ───────────────────────── laço de nível superior ───────────────────────── */
static void repl(void) {
    for (;;) {
        in = tty; sig = SIG_NONE; depth = 0; bsp = 0; lexpr_sp = 0;
        L x = read_obj();
        if (x == EOF_OBJ) break;
        if (sig) { fprintf(stderr, ";%s\n", err_msg); sig = SIG_NONE; continue; }
        L v = eval(x);
        if (sig == SIG_ERR) { fprintf(stderr, ";%s", err_msg); if (err_obj && err_obj != NIL) { fputs(" - ", stderr); FILE *so = out; out = stderr; prin1(err_obj); out = so; } fputc('\n', stderr); sig = SIG_NONE; continue; }
        if (sig) { fprintf(stderr, ";sinal %d fora de lugar\n", sig); sig = SIG_NONE; continue; }
        terpri(); prin1(v); terpri(); fflush(out);
    }
}

int main(int argc, char **argv) {
    init();
    /* Trace isolado em arquivo: nunca interfere com a saida PTY do dialogo.
       Ativado apenas quando o operador fornece um caminho explicitamente. */
    const char *f25_path=getenv("ELCIA_F25_DISPATCH_TRACE");
    if (f25_path && *f25_path) {
        f25_fp=fopen(f25_path,"w");
        if (!f25_fp) { perror("ELCIA_F25_DISPATCH_TRACE"); return 2; }
    }
    const char *f24_path=getenv("ELCIA_F24_OBJECT_TRACE");
    if (f24_path && *f24_path) {
        f24_fp=fopen(f24_path,"w");
        if (!f24_fp) { perror("ELCIA_F24_OBJECT_TRACE"); return 2; }
    }
    const char *f23_path=getenv("ELCIA_F23_DB_TRACE");
    if (f23_path && *f23_path) {
        f23_db_fp=fopen(f23_path,"w");
        if (!f23_db_fp) { perror("ELCIA_F23_DB_TRACE"); return 2; }
    }
    const char *watch_file=getenv("ELCIA_F22_WHO_TRACE");
    if (watch_file && *watch_file) {
        who_trace_fp=fopen(watch_file,"w");
        if (!who_trace_fp) { perror("ELCIA_F22_WHO_TRACE"); return 2; }
    }
    static Stream ttys = { NULL, 0, 0, 0, "TTY", 1, 1 }; tty = &ttys; in = tty;
    if (getenv("ECHO")) echo_tty = 1;
    int interactive = 0, total_err = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-i")) { interactive = 1; continue; }
        if (!strcmp(argv[i], "-v")) { verbose_load = 1; continue; }
        if (!strcmp(argv[i], "-b")) { trace_errors = 1; continue; }
        if (!strcmp(argv[i], "-A")) { trace_arity = 1; continue; }
        if (!strcmp(argv[i], "-e") && i + 1 < argc) {        /* avalia uma forma dada na linha de comando */
            Stream s = { argv[i + 1], 0, strlen(argv[i + 1]), 0, "-e", 1, 0 }; in = &s; sig = SIG_NONE; depth = 0; bsp = 0; lexpr_sp = 0;
            L x = read_obj(); in = tty; L v = eval(x);
            if (sig) { fprintf(stderr, ";%s\n", err_msg); sig = SIG_NONE; } else { prin1(v); terpri(); }
            i++; continue;
        }
        int e = load_file(argv[i], NULL); if (e > 0) total_err += e;
    }
    if (trace_arity) fprintf(stderr, ";ARITY_DIAG_SUMMARY mismatches=%llu printed=%d\n",
                             arity_mismatches, trace_arity_printed);
    fprintf(stderr, "avaliacoes: %ld  memoria: %ld KB  erros: %d\n", n_evals, n_alloc / 1024, total_err);
    if (interactive) repl();
    if (f25_fp) {
        fprintf(f25_fp,"#EVENTS\t%llu\n",f25_events);
        fclose(f25_fp);f25_fp=NULL;
    }
    if (f24_fp) {
        fprintf(f24_fp,"#EVENTS\t%llu\n",f24_events);
        fclose(f24_fp);f24_fp=NULL;
        free(f24_slots);f24_slots=NULL;
    }
    if (f23_db_fp) {
        fprintf(f23_db_fp,"#EVENTS\t%llu\n",f23_db_events);
        fclose(f23_db_fp);f23_db_fp=NULL;
    }
    if (who_trace_fp) {
        fprintf(who_trace_fp,"#EVENTS\t%llu\n",who_trace_events);
        fclose(who_trace_fp); who_trace_fp=NULL;
    }
    return total_err ? 1 : 0;
}
