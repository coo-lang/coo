%{
#include <string>
#include "ast.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)

int cur_line = 1;
int comment_nesting = 0;

void yyerror(const char *msg);

void unrecognized_char(char c);
void debug(char* msg);
%}
WHITESPACE   ([ \t\v\f\r]+)

SINGLE_COMMENT  ("//".*)
MULTIPLE_COMMENT_BEGIN ("/*")
MULTIPLE_COMMENT_END ("*/")

STRING_BEGIN ("\"")
STRING_END ("\"")

%x MULTIPLE_COMMENT
%x SINGLE_STRING

%%

{WHITESPACE}			;
[\n]                    {cur_line++; }

{SINGLE_COMMENT}                    ;
{MULTIPLE_COMMENT_BEGIN}            BEGIN(MULTIPLE_COMMENT);
<MULTIPLE_COMMENT>{
  \n                                cur_line++;
  {MULTIPLE_COMMENT_BEGIN}          comment_nesting++;
  {MULTIPLE_COMMENT_END}            { if (comment_nesting) --comment_nesting;
                                      else BEGIN(INITIAL); }
  <<EOF>>                           yyerror("the comment misses */ to termiate before EOF");
  .                                 ;
}

"true"|"false"              SAVE_TOKEN; return TBOOLLIT;

[a-zA-Z_][a-zA-Z0-9_]*	    SAVE_TOKEN; return TIDENTIFIER;
[0-9]+(\.[0-9]*[fF]?|[fF])	SAVE_TOKEN; return TDOUBLELIT;
[0-9]+					            SAVE_TOKEN; return TINTEGERLIT;
[0-9]+[lL]                  SAVE_TOKEN; return TLONGLIT;

{STRING_BEGIN}              BEGIN(SINGLE_STRING); printf("str start: %d\n", yy_start);
<SINGLE_STRING>{
  \n                        yyerror("the string misses \" to termiate before newline");
  <<EOF>>                   yyerror("the string misses \" to terminate before EOF");
  ([^\\\"]|\\.)*            SAVE_TOKEN;
  {STRING_END}              BEGIN(INITIAL); return TSTRINGLIT;
  .                         unrecognized_char(yytext[0]);
}

"="						return TOKEN(TEQUAL);
"=="					return TOKEN(TCEQ);
"!="					return TOKEN(TCNE);
"<"						return TOKEN(TCLT);
"<="					return TOKEN(TCLE);
">"						return TOKEN(TCGT);
">="					return TOKEN(TCGE);
"("						return TOKEN(TLPAREN);
")"						return TOKEN(TRPAREN);
"{"						return TOKEN(TLBRACE);
"}"						return TOKEN(TRBRACE);
"."						return TOKEN(TDOT);
","						return TOKEN(TCOMMA);
"+"						return TOKEN(TPLUS);
"-"						return TOKEN(TMINUS);
"*"						return TOKEN(TMUL);
"/"						return TOKEN(TDIV);

<<EOF>>       return 0;

.						  unrecognized_char(yytext[0]);

%%

void debug(char* msg) {
    printf("%s(%d): %d: %s\n", yytext, yyleng, cur_line, msg);
}

void unrecognized_char(char c) {
    char buf[32] = "Unrecognized character: ?";
    buf[24] = c;
    yyerror(buf);
}

void yyerror(const char *msg) {
    fprintf(stderr, "Error at line %d:\n\t%s\n", cur_line, msg);
    exit(-1);
}

int yywrap(void) {
    return 1;
}