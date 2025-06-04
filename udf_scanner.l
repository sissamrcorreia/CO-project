%option c++ prefix="udf_scanner_" outfile="udf_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{
// make relevant includes before including the parser's tab file
#include <string>
#include <cstring>
#include <vector>
#include <errno.h>
#include <sstream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "ast/tensor_node.h"
#include "udf_parser.tab.h"

// output stream for building string literals
static std::ostringstream strlit;

#define SAFE_STRTOUL(base) {                \
  int i = std::strtoul(yytext, NULL, base); \
  if (errno == ERANGE) {                    \
    yyerror("Integer overflow");            \
    errno = 0;                              \
  } else {                                  \
    yylval.i = i;                           \
    return tINTEGER;                        \
  }                                         \
}

#define SAFE_STRTOD() {                 \
  double d = std::strtod(yytext, NULL); \
  if (errno == ERANGE) {                \
    yyerror("Real overflow");           \
    errno = 0;                          \
  } else {                              \
    yylval.d = d;                       \
    return tREAL;                       \
  }                                     \
}

// don't change this
#define yyerror LexerError
%}

SPACE     ([ \t\r]|\n)
INT       [[:digit:]]+

LETTER  [A-Za-z_]
IDENTIFIER   {LETTER}({LETTER}|[[:digit:]]|_)*

%x X_COMMENT
%x X_STRING  X_BACKSLASH

%%
  yydebug=1; set_debug(1);

  /* ====================================================================== */
  /* ====[                       COMMENTS                             ]==== */
  /* ====================================================================== */

"//".*$                /* match the whole line (it is ignored) */

"/*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"/*"        yy_push_state(X_COMMENT);
<X_COMMENT>"*/"        yy_pop_state();
<X_COMMENT>.|"\n"      ;  /* ignore inside comments */

  /* ====================================================================== */
  /* ====[                        Data Types                          ]==== */
  /* ====[                        Null pointers                       ]==== */
  /* ====================================================================== */

"int"                    return tTYPE_INT;
"real"                   return tTYPE_REAL;
"string"                 return tTYPE_STRING;
"void"                   return tTYPE_VOID;
"tensor"                 return tTYPE_TENSOR;
"auto"                   return tTYPE_AUTO;
"ptr"                    return tTYPE_POINTER;
[<>]                     return *yytext;

"nullptr"                return tNULLPTR;
"sizeof"                 return tSIZEOF;
"objects"                return tOBJECTS;

  /* ====================================================================== */
  /* ====[                     Global Symbols                         ]==== */
  /* ====================================================================== */

"public"                 return tPUBLIC;
"forward"                return tFORWARD;

  /* ====================================================================== */
  /* ====[                     Conditional Statements                 ]==== */
  /* ====================================================================== */

"if"                     return tIF;
"else"                   return tELSE;
"elif"                   return tELIF;

  /* ====================================================================== */
  /* ====[                     Iteration Statements                   ]==== */
  /* ====[                     Termination Statements                 ]==== */
  /* ====[                     Continuation Statements                ]==== */
  /* ====================================================================== */

"for"                    return tFOR;
"break"                  return tBREAK;
"continue"               return tCONTINUE;

  /* ====================================================================== */
  /* ====[                     Expression Operators                   ]==== */
  /* ====================================================================== */

[\[\](),?]             return *yytext;
[-+*/%~<>@?=]          return *yytext;
"=="                   return tEQ;
"!="                   return tNE;
"<="                   return tLE;
">="                   return tGE;
"&&"                   return tAND;
"||"                   return tOR;

"."                    return *yytext;
"**"                   return tCONTRACT;
"capacity"             return tCAPACITY;
"dim"                  return tDIM;
"dims"                 return tDIMS;
"rank"                 return tRANK;
"reshape"              return tRESHAPE;
"input"                return tINPUT;

  /* ====================================================================== */
  /* ====[                     Delimiters and Separators              ]==== */
  /* ====[                     Function Body                          ]==== */
  /* ====[                     Return Statement                       ]==== */
  /* ====================================================================== */

[#\[\](){};:!,]                return *yytext;
"return"                       return tRETURN;
"write"                        return tWRITE;
"writeln"                      return tWRITELN;

  /* ====================================================================== */
  /* ====[                     Integers                               ]==== */
  /* ====================================================================== */

0x[[:xdigit:]]+           SAFE_STRTOUL(16);
0x[[:alnum:]]+            yyerror("Invalid base 16 number");

0|[1-9][0-9]*             SAFE_STRTOUL(10);

  /* ====================================================================== */
  /* ====[                     Reais                                  ]==== */
  /* ====================================================================== */

([0-9]*\.[0-9]+|[0-9]+\.[0-9]*)([Ee][-+]?[0-9]+)? SAFE_STRTOD();

  /* ====================================================================== */
  /* ====[                     Identifiers (names)                    ]==== */
  /* ====================================================================== */

{IDENTIFIER}             { yylval.s = new std::string(yytext); return tIDENTIFIER; }

  /* ====================================================================== */
  /* ====[                     Strings                                ]==== */
  /* ====================================================================== */

"\""                           yy_push_state(X_STRING);
<X_STRING>\\                   yy_push_state(X_BACKSLASH);
<X_STRING>"\""                 {
                                 yylval.s = new std::string(strlit.str());
                                 strlit.str("");
                                 yy_pop_state();
                                 return tSTRING;
                               }
<X_STRING>.                    strlit << *yytext;

<X_BACKSLASH>n                 strlit << '\n'; yy_pop_state();
<X_BACKSLASH>r                 strlit << '\r'; yy_pop_state();
<X_BACKSLASH>t                 strlit << '\t'; yy_pop_state();
<X_BACKSLASH>"\""              strlit << '"';  yy_pop_state();
<X_BACKSLASH>\\                strlit << '\\'; yy_pop_state();

<X_BACKSLASH>[[:xdigit:]]{1,2} {
                                 strlit << (char)(unsigned char)strtoul(yytext, NULL, 16);
                                 yy_pop_state();
                               }
<X_BACKSLASH>.                 strlit << *yytext; yy_pop_state();

  /* ====================================================================== */
  /* ====[                    (other characters)                      ]==== */
  /* ====================================================================== */

[[:space:]]         /* ignore remaining white space inside the program */
.                      yyerror(yytext);    /* all other chars: error! */

%%
