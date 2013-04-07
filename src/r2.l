%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
extern char *malloc();
#define COPY_TEXT yylval.sval = strdup(yytext)
int lineno = 1;
int comment=0;
%}
letter [A-Za-z_]
dig [0-9]
hexdig ([a-fA-F]|{dig})
ident {letter}({letter}|{dig})*
lop	"<<"|"+<<"|"<-"|"&&"|"||"|"<="|"!="|">="|"+="|"-="|"*="|":="|"::="|"=="|"="|"<"|">"|"+"|"-"|"*"|"/"|"^"|"%"|"?"
rop ">>"|"->"|">>+"|"+>>"|"-="|"+"|"=*"
qstring \"[^"\n]*\"
mqstring \"(\\\"|[^"\n])*\"
qchar \'[^'"]*\'
%%
\n			lineno++;	/* keep track of current line number */
\/\/.*		;		/* ignore c++ comment */
#.*		;		/* ignore shell-type comment */
\/\*	comment++;	/* begin comment */
\*\/		{	/* end comment */
				if (comment > 0)
					comment--;
				else
				{
					fprintf(stderr, "Error, line %d: Mismatched \"*/\".\n", lineno);
					exit(-1);
				}
			}
begin		if (!comment) return _BEGIN;
end			if (!comment) return _END;
if			if (!comment) return _IF;
else		if (!comment) return _ELSE;
while		if (!comment) return _WHILE;
return		if (!comment) return _RETURN;
break		if (!comment) return _BREAK;
continue	if (!comment) return _CONTINUE;
exit		if (!comment) return _EXIT;
file		if (!comment) return A_FILE;
true|false	{ if (!comment) { COPY_TEXT; return _BOOL;} }
{ident}		{ if (!comment) { COPY_TEXT; return ID; } }
{mqstring}	{ if (!comment) { COPY_TEXT; return QSTRING;} }
{qchar}		{
				if (!comment)
				{
					yylval.sval = (char *) malloc(sizeof(char)+1);
					*(yylval.sval) = yytext[1]; 
					yylval.sval[1] = '\0'; return QCHAR;
				}
			}
"::"		if (!comment) return SCOPE;
-?{dig}+		{ if (!comment) COPY_TEXT; return INT; /* text of the number */ }
-?({dig}*\.{dig}+)([eE][-+]?{dig}+)?	{ if (!comment) { COPY_TEXT; return FLOAT; } }
0x{hexdig}+	{ if (!comment) { COPY_TEXT; return HEXVAL; } }
{lop}|{rop}		{ if (!comment) { COPY_TEXT; return LOP; } }
"++"|"--"|"!"	{ if (!comment) { COPY_TEXT; return UNARYOP; } }
";"			if (!comment) { return SEMI; }
[ \n\t]		;		/* ignore whitespace */
.			if (!comment) { return yytext[0];	/* any other unmatched single char */ }
%%

