/* Definition header for resulting compiler*/

%{

#include <stdio.h>
#include "ast.h"
NBlock *programBlock;

extern int yylex();
void yyerror(const char *s);

%}

/* Represents the different ways to access our code being compiled*/

%union {
	Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
	NVariableDeclaration *var_decl;
	std::vector<NVariableDeclaration*> *varvec;
	std::vector<NExpression*> *exprvec;
	std::string *string;
	int token;
}

/* Terminal symbols. They need to match tokens in tokens.l file */

%token <string> TIDENTIFIER TINTEGERLIT TDOUBLELIT TLONGLIT TBOOLLIT TSTRINGLIT
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT TCOLON TSEMICOLON
%token <token> TPLUS TMINUS TMUL TDIV
/* keywords */
%token <token> TVAR TDEF TIF TELSE TFOR TRET

/* Non Terminal symbols. Types refer to union decl above */
%type <ident> ident
%type <expr> numeric boolean string expr
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl_arg func_decl if_stmt for_stmt ret_stmt
%type <token> comparison

/* Operator precedence */
%left TPLUS TMINUS
%left TMUL TDIV

/* Starting rule in the grammar*/

%start program

/* Grammar rules */

%%

program: stmts { programBlock = $1; }
	;

stmts: { $$ = new NBlock();  }
	| stmts stmt { $1->statements.push_back($<stmt>2); }
	;

stmt: var_decl | func_decl
	| expr { $$ = new NExpressionStatement(*$1); }
	| ret_stmt
	| if_stmt
	| for_stmt
	;

block: TLBRACE stmts TRBRACE { $$ = $2; }
	| TLBRACE TRBRACE { $$ = new NBlock(); }
	;

var_decl: TVAR ident TCOLON ident { $$ = new NVariableDeclaration(*$4, *$2); }
		| TVAR ident TCOLON ident TEQUAL expr { $$ = new NVariableDeclaration(*$4, *$2, $6); }
		;

func_decl: TDEF ident TLPAREN func_decl_args TRPAREN TCOLON ident block
			{ $$ = new NFunctionDeclaration(*$7, *$2, *$4, *$8); delete $4; }
		;

func_decl_arg: ident TCOLON ident { $$ = new NVariableDeclaration(*$3, *$1); }
			| ident TCOLON ident TEQUAL expr { $$ = new NVariableDeclaration(*$3, *$1, $5); }
			;

func_decl_args: /* Blank! */ {$$ = new VariableList(); }
			| func_decl_arg { $$ = new VariableList(); $$->push_back($<var_decl>1); }
			| func_decl_args TCOMMA func_decl_arg { $1->push_back($<var_decl>3); }
			;

if_stmt: TIF expr block	{printf("parsing if block\n"); $$ = new NIfStatement(*$2, *$3); }
	| TIF expr block TELSE block {printf("parsing if else block\n"); $$ = new NIfStatement(*$2, *$3, *$5); }
	;

for_stmt: TFOR expr TSEMICOLON expr TSEMICOLON expr block {printf("parsing for block\n"); $$ = new NForStatement(*$2, *$4, *$6, *$7); }
	;

ident: TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
	;

numeric: TINTEGERLIT { $$ = new NInteger(atoi($1->c_str())); delete $1; }
	| TLONGLIT {$$ = new NLong(atol($1->c_str())); delete $1; }
	| TDOUBLELIT { $$ = new NDouble(atof($1->c_str())); delete $1; }
	;

boolean: TBOOLLIT {$$ = new NBoolean($1->c_str()[0] == 't'); delete $1; }
	;

string: TSTRINGLIT {$$ = new NString(*$1); delete $1; }
	;

expr: ident TEQUAL expr { $$ = new NAssignment(*$<ident>1, *$3); }
	| ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
	| expr TPLUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
	| expr TMINUS expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
	| expr TMUL expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
	| expr TDIV expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
	| expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
	| TMINUS expr {$$ = new NUnaryOperator($1, *$2); }
	| TLPAREN expr TRPAREN { $$ = $2; }
	| ident { $<ident>$ = $1; }
	| numeric
	| boolean
	| string
	;

ret_stmt: TRET expr	{printf("ret expr"); $$ = new NRet(*$2); }
		;

call_args: /* Blank! */ { $$ = new ExpressionList(); }
	    | expr { $$ = new ExpressionList(); $$->push_back($1); }
	    | call_args TCOMMA expr { $1->push_back($3); }
        ;

comparison: TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE
		;

%%
