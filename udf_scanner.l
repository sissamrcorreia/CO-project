%option c++ prefix="udf_scanner_" outfile="udf_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{
// make relevant includes before including the parser's tab file
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "ast/tensor_node.h"
#include "udf_parser.tab.h"

static std::vector<int> deferred;  // deferred tokens

// output stream for building string literals
static std::ostringstream strlit;

// don't change this
#define yyerror LexerError
%}

SPACE     ([ \t\r]|\n)
INT       [[:digit:]]+

LETTER  [A-Za-z_]
IDENTIFIER   {LETTER}({LETTER}|[[:digit:]])*

%x X_COMMENT
%x X_STRING  X_BACKSLASH
%x X_OCTAL   X_HEXADECIMAL X_HEX_INT X_BIN_INT

%%

  /* ====================================================================== */
  /* ====[                       COMMENTS                             ]==== */
  /* ====================================================================== */

"//".*$                /* match the whole line (it is ignored) */
"/*"                   yy_push_state(X_COMMENT);
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

"**"                   return tCONTRACT;
".capacity"            return tCAPACITY;
".dim"                 return tDIM;
".dims"                return tDIMS;
".rank"                return tRANK;
".reshape"             return tRESHAPE;
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

"0x"                   yy_push_state(X_HEX_INT);

<X_HEX_INT>[[:xdigit:]]+      { yylval.i = strtoul(yytext, NULL, 16); yy_pop_state(); return tINTEGER; }
<X_HEX_INT>.|\n               std::cerr << "WARNING: line " << lineno() << ": bad hexadecimal data!" << std::endl; exit(1);

{INT}                  yylval.i = strtoul(yytext, NULL, 10); return tINTEGER;

  /* ====================================================================== */
  /* ====[                     Reais                                  ]==== */
  /* ====================================================================== */


([0-9]*\.[0-9]+|[0-9]+\.[0-9]*)([eE][-+]?[0-9]+)? { yylval.d = strtod(yytext, NULL); return tREAL; }

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
