%{

/* r2.y
 * the RSL yacc specification
 *
 * $Id: r2.y,v 1.6 1996/03/18 13:29:52 holtrf Exp $
 *
 * History:
 *
 * October 1995 - initial version
 *
 * 12/7/95- removing memory leaks between lexer/parser/rsl
 *   - rc_init() no longer strdup()'s because this copy was
 *     done in the lexer in most cases. That means to send
 *     a literal string to rc_init() you must put a strdup()
 *     inline, like rc_init(rc, TYPE_RESCALL, $1, strdup("[]"));
 *     otherwise free() will try to free non-heap memory
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "r2.h"

int parse_it();
struct res_call *rc_create();
void rc_init();
void rc_kill();
void rc_kill_list();
void rc_ins_args();
struct res_call *rc_newvar();
void rc_print();
void rc_printlist();
void warning();
int yyerror(char *);

#define OUT_OF_MEMORY "Out of memory"
#define MEM_ERROR 9
#define ERROR 1
#define NO_ERROR 0
#define DO_MEM_ERR yyerror((error_level=MEM_ERROR, OUT_OF_MEMORY))

extern struct res_call *the_prog;
extern int lineno;
int nerrors=0, _done_=0, error_level= NO_ERROR;
extern int Cres_call_creates, Cres_call_destroys;

#define FILENAME_LENGTH 256
char CurrentFilename[FILENAME_LENGTH];

%}

%union  {
	char *sval;
	struct res_call *resval;
}

%token <sval> ID,QSTRING,INT,FLOAT,HEXVAL,LOP,ROP,QCHAR,UNARYOP,_BOOL
%token _BEGIN _END SEMI A_FILE _IF _ELSE SCOPE _WHILE _RETURN _BREAK _CONTINUE _EXIT

%type <resval> s_list,statement,expr_list,decl,decl_list,named_compound,compound,program
%type <resval> assign_expr,unary_expr,postfix_expr,primary_expr,literal

%nonassoc LOWER_THAN_ELSE
%nonassoc _ELSE

%%

program: ID compound
		{
			struct res_call *rc = $2;
			rc->res_name = $1;		/* strdup($1);name the compound */
			rc->method_name = NULL;	/* make sure */
			rc->next = NULL; 	/* the_prog; */
			the_prog = rc;
#ifdef DEBUG
			printf("recognized compound %s\n", $1);
#endif
			$$ = rc;
		}
	| ID SCOPE ID compound
		{
			struct res_call *rc = $4;
			rc->res_name = 	$1; /* strdup($1);	 class-object name */
			rc->method_name = $3;	/* strdup($3);"method" name */
			rc->next = NULL; 	/* the_prog; */
			the_prog = rc;
#ifdef DEBUG
			printf("recognized scoped compound %s :: %s\n", $1, $3);
#endif
			$$ = rc;
		
		}
	| program ID compound
		{
			struct res_call *rc = $3; /* compound */
			rc->res_name = $2;		/* strdup($2);name the compound */
			rc->method_name = NULL;	/* make sure */
			rc->next = $1;
			the_prog = rc;
#ifdef DEBUG
			printf("recognized program and compound %s\n", $2);
#endif
			$$ = rc;
		}
	| program ID SCOPE ID compound
		{
			struct res_call *rc = $5; /* compound */
			rc->res_name = $2;		/* strdup($2);class-object name */
			rc->method_name = $4;	/* strdup($4);method name */
			rc->next = $1;
			the_prog = rc;
#ifdef DEBUG
			printf("recognized program and scoped compound %s :: %s\n", $2, $4);
#endif
			$$ = rc;
		}
	;

named_compound: ID compound
		{
			struct res_call *rc = $2; /* compound */
			if (rc)
				rc->res_name = $1;		/* strdup($1);name the compound */
			$$ = rc;
			/* | ID '(' decl_list ')' compound */
		}
	| ID SCOPE ID compound
	{
		struct res_call *rc = $4; /* compound */
		if (rc) {
			rc->res_name = $1;		/* strdup($1);class-object name */
			rc->method_name = $3;	/* strdup($3);method name */
		}
		$$ = rc;
		/* | ID SCOPE ID '(' decl_list ')' compound */
	}

compound: bb s_list eb
		{
			struct res_call *rc = rc_create();
			rc_init(rc, TYPE_RESTABLE, NULL, NULL);	/* no name, method */
			rc->next = NULL;
			rc->args = $2;	/* the statement list to be executed */
			$$ = rc;
		}
	;

bb:	_BEGIN | '{'
	;
eb: _END | '}'
	;
		
s_list: statement { $$ = $1; }
	| s_list statement
		{
			if ($2) {
				$2->next = $1;
				$$ = $2;
			} else			/* error - statement came back null */
				$$ = $1;	/* don't lose s_list! */
		}
	;

statement: assign_expr SEMI { $$ = $1; }
	| compound { $$ = $1; }
	| _IF '(' assign_expr ')' statement		%prec LOWER_THAN_ELSE
		{
			struct res_call *rc = rc_create();
			rc_init(rc, TYPE_SELECT, NULL, NULL);	/* no name or method */
			rc->eval = $3;	/* expr to evaluate */
			rc->args = $5;	/* then condition */
			$$ = rc;
		}
	| _IF '(' assign_expr ')' statement _ELSE statement
		{
			struct res_call *rc = rc_create();
			rc_init(rc, TYPE_SELECT, NULL, NULL);	/* no name or method */
			rc->eval = $3;	/* expr to evaluate */
			rc->args = $5;	/* then condition */
			rc->extra = $7;	/* else condition */
			$$ = rc;
		}
	| decl SEMI
		{	/* declaration */
			$$ = $1;
		}
	| _WHILE '(' assign_expr ')' statement
		{
			struct res_call *rc = rc_create();
			if (rc)
			{
				rc_init(rc, TYPE_LOOP, NULL, NULL);	/* no name or method */
				rc->eval = $3;	/* expression to evaluate */
				rc->args = $5;	/* statement to repeat */
			}
			$$ = rc;
		}
	| _RETURN SEMI
		{
			/* Soon: allow a return value */
			struct res_call *rc = rc_create();
			if (rc)
				rc_init(rc, TYPE_RETURN, NULL, NULL);
			$$ = rc;
		}
	| _BREAK SEMI
		{
			struct res_call *rc = rc_create();
			if (rc)
				rc_init(rc, TYPE_BREAK, NULL, NULL);
			$$ = rc;
		}
	| _CONTINUE SEMI
		{
			struct res_call *rc = rc_create();
			if (rc)
				rc_init(rc, TYPE_CONTINUE, NULL, NULL);
			$$ = rc;
		}
	| _EXIT SEMI
		{
			struct res_call *rc = rc_create();
			if (rc)
				rc_init(rc, TYPE_EXIT, NULL, NULL);
			$$ = rc;
		}
	| assign_expr error SEMI
		{
			/* error token */
			yyerror("missing ';'\n");	/* at or before line %d\n", lineno); */
			/* exit(1); */
			$$ = NULL;
		}
	;


assign_expr: unary_expr
		{
			if ($1 == NULL)
			{
				yyerror("unary expr is NULL\n");
				exit(1);
			}
			$$ = $1;
#ifdef DEBUG
	printf(" assign_expr ---> unary_expr\n");
#endif
		}
	| unary_expr LOP assign_expr
	{
		struct res_call *rc = $1;
#ifdef DEBUG
	printf(" assign_expr for %s recognized\n", $2);
#endif
		if (rc)
		{
			if (rc->type  == TYPE_VAR_REF
				
					) /* != TYPE_RESCALL) */
			{
				rc->method_name = $2;
				rc->args = $3;
				rc->type = TYPE_RESCALL;
			}
			else
			{
				rc = rc_create();
				rc_init(rc, TYPE_EVAL, NULL, $2);
				rc->eval = $1;
				rc->args = $3;
			}
		}
		$$ = rc;
	}
	;

unary_expr: postfix_expr
		{
			$$ = $1;
#ifdef DEBUG
	printf("  postfix_expr recognized\n");
#endif
		}
	| UNARYOP postfix_expr	/* unary_expr */
		{
			struct res_call *rc = rc_create();
			rc_init(rc, TYPE_EVAL, NULL, $1);
			if (rc)
			{
				if ($2->type == TYPE_VAR_REF)
				{
					rc->res_name = strdup($2->res_name);
					rc->method_name = $1;
					rc->type = TYPE_RESCALL;
					/* rc->args = $2; */
					rc_kill($2);
				}
				else
					rc->eval = $2;	/* this doesn't work */
			}
			$$ = rc;
		}
	| A_FILE primary_expr
		{
			struct res_call *rc = rc_create();
			rc_init(rc, TYPE_FILE, NULL, NULL);
			if (rc) rc->args = $2;
			$$ = rc;
		}
	;

postfix_expr: primary_expr
		{
			$$ = $1;
#ifdef DEBUG
	printf("   primary_expr recognized\n");
#endif
		}
	| ID '[' assign_expr ']'
		{	/* index - could be postfix_expr instead of ID */
			struct res_call *rc = rc_create();
			rc_init(rc, TYPE_RESCALL, $1, strdup("[]"));
			if (rc) rc->args = $3;
			$$ = rc;
		}
	| ID '(' expr_list ')'
		{	/* function call -- a() means object 'a' and method 'a' */
			struct res_call *rc = rc_create();
#ifdef DEBUG
	printf("   function call: %s()\n", $1);
#endif
				/* resource and method names are the same */
			rc_init(rc, TYPE_RESCALL, $1, $1);
			if (rc)
				rc->args = $3;
			else
				DO_MEM_ERR;	/* ugg */
			$$ = rc;
		}
	| ID '.' ID '(' expr_list ')'
		{	/* object.method() invocation */
			struct res_call *rc = rc_create();
#ifdef DEBUG
	printf("   object.method: %s.%s()\n", $1, $3);
#endif
			if (rc) {
				rc_init(rc, TYPE_RESCALL, $1, $3);
				rc->args = $5;
			}
			else
				DO_MEM_ERR;
			$$ = rc;
		}
	| literal '.' ID '(' expr_list ')'
		{	/* literal-object.method() invocation */
			struct res_call *rc = rc_create();
#ifdef DEBUG
	printf("   literal-object.method: %s.%s()\n", $1, $3);
#endif
			if (rc) {
				rc_init(rc, TYPE_RESCALL, $1, $3);	/* null resname */
				rc->eval = $1;	/* "evaluate" the first one */
				rc->args = $5;
			}
			else
				DO_MEM_ERR;
			$$ = rc;
		}
	| ID '.' LOP '(' expr_list ')'
		{	/* object.method() invocation */
			struct res_call *rc = rc_create();
#ifdef DEBUG
	printf("   object.op-method: %s.%s()\n", $1, $3);
#endif
			if (rc) {
				rc_init(rc, TYPE_RESCALL, $1, $3);
				rc->args = $5;
			}
			else
				DO_MEM_ERR;
			$$ = rc;
		}
	| literal '.' LOP '(' expr_list ')'
		{	/* object.method() invocation */
			struct res_call *rc = rc_create();
#ifdef DEBUG
	printf("   literal-object.op-method: %s.%s()\n", $1, $3);
#endif
			if (rc) {
				rc_init(rc, TYPE_RESCALL, "", $3);	/* null resname */
				rc->eval = $1;	/* "evaluate" the first one */
				rc->args = $5;
			}
			else
				DO_MEM_ERR;
			$$ = rc;
		}
	| ID '.' ID
		{
			/* object member variable reference */
			struct res_call *rc = rc_create();
			
			if (rc)
				rc_init(rc, TYPE_OBJ_VAR_REF, $1, $3);
			else
				DO_MEM_ERR;
#ifdef DEBUG
			printf("   object.variable: %s.%s\n", $1, $3);
#endif
			$$ = rc;
		}
	| postfix_expr UNARYOP
		{	/* ++ or -- */
			struct res_call *rc = rc_create();
			if (rc) {
				rc_init(rc, TYPE_EVAL, NULL, $2);
				
				if ($1->type == TYPE_VAR_REF)
				{
					rc->res_name = strdup($1->res_name);
					rc->method_name = $2;
					rc->type = TYPE_RESCALL;
					/* rc->args = $2; */
					rc_kill($1);
				}
				else
					rc->eval = $1;	/* this doesn't work */
			}
			$$ = rc;
		}
	;

primary_expr: literal
		{
			$$ = $1;
#ifdef DEBUG
	printf("    literal\n");
#endif
		}
	| ID ':' literal
		{	/* "parameter" tagging */
			struct res_call *rc = $3;
			if (rc)
				rc->res_name = $1;
			$$ = rc;
		}
	| '(' assign_expr ')' { $$ = $2; }
	| '(' assign_expr error ')'
		{
			printf("  - missing ')'\n");
			yyerrok;
			$$ = $2;
		}
	| ID
		{
			struct res_call *rc = rc_create();
			if (rc) {
				rc_init(rc, TYPE_VAR_REF, $1, NULL);
#ifdef DEBUG
				printf("    ID: %s\n", $1);
#endif
			}
			else
				DO_MEM_ERR;
			$$ = rc;
		}
	;

literal: QSTRING
		{
			struct res_call *rc = rc_newvar($1, TYPE_STRING);
#ifdef DEBUG
	printf("     String: %s\n", $1);
#endif
			if (!rc)
				DO_MEM_ERR;
			$$ = rc;
		}
	| QCHAR
		{
			struct res_call *rc = rc_newvar($1, TYPE_STRING);
			if (!rc)
				DO_MEM_ERR;
			$$ = rc;
		}
	| INT
		{
			struct res_call *rc = rc_newvar($1, TYPE_INT);
			if (!rc)
				DO_MEM_ERR;
			$$ = rc;
		}
	| FLOAT
		{
			struct res_call *rc = rc_newvar($1, TYPE_FLOAT);
			if (!rc)
				DO_MEM_ERR;
			$$ = rc;
		}
	| HEXVAL
		{
			struct res_call *rc = rc_newvar($1, TYPE_HEXVAL);
			if (!rc)
				DO_MEM_ERR;
			$$ = rc;	
		}
	| _BOOL
		{
			struct res_call *rc = rc_newvar($1, TYPE_BOOLEAN);
			if (!rc)
				DO_MEM_ERR;
			$$ = rc;
		}
	;



expr_list:
		/* null */	{ $$ = NULL; }
	|	assign_expr /* unary_expr */	{ $$ = $1; }
	|	expr_list ',' assign_expr /* unary_expr */
		{
			struct res_call *rc = $3;
			if (rc)
				rc->next = $1;	/* add the expression to the list */
			$$ = rc;
		}
	;

decl: ID decl_list
		{	/* declaration: type name_1, name_2, ..., name_n;
			 * is translated to a rescall to the template object
			 * of type "type" and name "type", with method
			 * "@ Create" (so users can't call it directly from a script),
			 * and a list of names of the objects to create. */
			struct res_call *rc = rc_create();
			if (rc) {
				rc_init(rc, TYPE_RESCALL, $1, strdup("@ Create"));
				rc->args = $2;	/* add declaration list */
			} else
				DO_MEM_ERR;
			$$ = rc;
		}
	;

decl_list: ID
		{
			struct res_call *rc = rc_create();
			if (rc)
				rc_init(rc, TYPE_DECL, $1, NULL);
			else
				DO_MEM_ERR;
			$$ = rc;
		}
	| decl_list ',' ID
		{
			struct res_call *rc = rc_create();
			/*	rc_newvar($3, TYPE_DECL);	*/
			if (rc) {
				rc_init(rc, TYPE_DECL, $3, NULL);
				rc->next = $1;
			}
			else
				DO_MEM_ERR;
			$$ = rc;
		}
	;
	
%%

extern FILE *yyin;
extern char *yytext;
int parse_it(fn)
char *fn;
{
	lineno = 0;

	if (!fn) {
		fprintf(stderr, "Error: no file to parse.\n");
		return 1;
	}
	if (fn && fn[0] == '\0')
	{
		yyin = stdin;
	}
	else
	{
		strncpy(CurrentFilename, fn, FILENAME_LENGTH);
		yyin = fopen(fn, "r");
	}

	if (!yyin)
		return 1;

	while(!feof(yyin) && !_done_)
		yyparse();

/*	
	printf("\nrecognized program:\n");
	rc_printlist(the_prog, "\n");
	printf("\n\n\n");
*/
	return nerrors;

}

yyerror(msg)
char *msg;
{
	fprintf(stderr, "%s, line %d: %s\n", CurrentFilename, lineno, msg);
	if (nerrors++ > 5)
	{
		fprintf(stderr, "%s: Too many errors, exiting..\n\n", CurrentFilename);
		exit(-1);
	}
}

void warning(m)
char *m;
{
	fprintf(stderr, "Warning: %s\n", m);
}

/* allocate memory for a res_call and initialize pointers */
struct res_call *rc_create()
{
struct res_call *rc;
	rc = (struct res_call *) malloc(sizeof(struct res_call));
	if (rc) {
		rc->next = NULL;	
		rc->args = NULL;
		rc->type = TYPE_RESCALL;
		rc->method_name = NULL;
		rc->res_name = NULL;
		rc->extra = NULL;
		rc->eval = NULL;
		Cres_call_creates++;
#ifdef MEMCHECK
		fprintf(stderr, "allocate, ");
#endif
	}
	return rc;
}

/* create a new res_call for a variable reference or
 * a string resource (from a string literal) */
struct res_call *rc_newvar(value, type)
char *value;
int type;
{
struct res_call *r = rc_create();
	if (r)
		rc_init(r, type, NULL, value);
	return r;
}

/* initialize a res_call with arguments, some or all
 * of which could be NULL
 *
 * 12/7/95 - removed strdup()'s
 */
void rc_init(r, t, resname, methname)
struct res_call *r;
int t;
char *resname;
char *methname;
{
	if (!r) return;
	r->type = t;
	if (resname) {
		r->res_name = resname;	/* strdup(resname); */
	}
	if (methname) {
		r->method_name = methname;	/* strdup(methname); */
	}
	r->eval = NULL;
	r->extra = NULL;
#ifdef MEMCHECK
	fprintf(stderr, "initialize type %d\n", t);
#endif
}

/* insert arguments to a rescall */
void rc_ins_args(r, a)
struct res_call *r, *a;
{
struct res_call *t;
	if (r && a)  {
		for(t=a; t->next; t=t->next)
			;	/* find end of a */
		t->next = r->args;
		r->args = a;
	}		
}


/* free a res_call
 * and its associated res_calls (except next!)
 */
void rc_kill(r)
struct res_call *r;
{
	if (!r) return;
#ifdef MEMCHECK
	fprintf(stderr, "free type %d", r->type);
	if (r->res_name)
		fprintf(stderr, ": %s", r->res_name);
	if (r->method_name)
		fprintf(stderr, " . %s", r->method_name);
	fprintf(stderr, "\n");
#endif

	if (r->res_name) {
		free(r->res_name);
		r->res_name = NULL;
	}
	if (r->method_name) {
		free(r->method_name);
		r->method_name = NULL;
	}
	if (r->args) {
		rc_kill_list(r->args);	/* free the arg list */
		r->args = NULL;
	}
	if (r->eval) {
		rc_kill_list(r->eval);	/* kill the whole list, if it is one */
		r->eval = NULL;
	}
	if (r->extra) {
		rc_kill_list(r->extra);
		r->extra = NULL;
	}
	free(r);
	r = NULL;
	Cres_call_destroys++;
}

/* recursively free a rescall list */
void rc_kill_list(r)
struct res_call *r;
{
struct res_call *t=r, *tnext=t;
	if (!r)
	{
#ifdef MEMCHECK
		printf("rc_kill_list: NULL argument.\n");
#endif
		return;
	}

	for(t=r; tnext; t = tnext)
	{
		tnext = t->next;
		rc_kill(t);
	}
}

/* print the contents of a res_call struct
 * doesn't print args
 */
void rc_print(r)
struct res_call *r;
{
	if (!r)
		return;
		

	switch(r->type)
	{
		case TYPE_RESCALL:	
			if (r->res_name)
				printf("[%d] %s", r->type, r->res_name);
			if (r->method_name)
				printf(".%s", r->method_name);
			printf("( ");
			if (r->args)
				rc_printlist(r->args, ", ");
			printf(" ) ");
			break;

		case TYPE_STRING:
		case TYPE_INT:
			if (r->method_name)
				printf("%s ", r->method_name);
			else
				printf("[str?] ");
			break;
			
		case TYPE_VAR_REF:	
			if (r->res_name)
				printf("%s ", r->res_name);
			else
				printf("[var?] ");
			break;

		case TYPE_DECL:		
			if (r->res_name)
				printf("%s ", r->res_name);
			else
				printf("[decl?] ");
			break;

		case TYPE_RESTABLE:
			if (r->res_name)
				printf("[%d] %s", r->type, r->res_name);
			printf(" {\n");
			if (r->args)
				rc_printlist(r->args, "\n");
			else
				printf("-- restable has no contents\n");
			printf("\n}\n");
			break;

		default: 			printf("[%d?]", r->type);
	}
	
	if (r->eval) {
		printf("eval: ");
		rc_print(r->eval);
	}

	if (r->extra) {
		printf("extra: ");
		rc_print(r->extra);
	}



	
}

/* recursively print a whole res_call list */
void rc_printlist(r, endl)
struct res_call *r;
char *endl;
{
struct res_call *t = r;
	while(t)
	{
		rc_print(t);
		t = t->next;
		if (t)
			printf("%s", endl);
	}
}
