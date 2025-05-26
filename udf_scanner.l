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
%x X_STRING  X_BACKSLASH   X_CHAR    X_CHAR_END
%x X_OCTAL   X_HEXADECIMAL X_HEX_INT X_BIN_INT

%%

  /* ====================================================================== */
  /* ====[                      2.2 - COMMENTS                        ]==== */
  /* ====================================================================== */

"--".*$                /* match the whole line (it is ignored) */
"/*"                   yy_push_state(X_COMMENT);
<X_COMMENT>"*/"        yy_pop_state();
<X_COMMENT>.|"\n"      ;  /* ignore inside comments */

  /* ====================================================================== */
  /* ====[                  1.1   - Data Types                        ]==== */
  /* ====[                  1.1   - Null pointers                     ]==== */
  /* ====================================================================== */

"int"                    return tTYPE_INT;
"real"                   return tTYPE_REAL;
"string"                 return tTYPE_STRING;
"tensor"                 return tTYPE_TENSOR;
"nullptr"                return tNULLPTR;
"sizeof"                 return tSIZEOF;

  /* ====================================================================== */
  /* ====[                  3.4.2 - Global Symbols                    ]==== */
  /* ====================================================================== */

"public"                 return tPUBLIC;
"forward"                return tFORWARD;

  /* ====================================================================== */
  /* ====[                 5.5 - Conditional Statements               ]==== */
  /* ====================================================================== */

"if"                   return tIF;
"else"                 return tELSE;

  /* ====================================================================== */
  /* ====[                 5.5 - Iteration Statements                 ]==== */
  /* ====[                 5.5 - Termination Statements               ]==== */
  /* ====[                 5.4 - Continuation Statements              ]==== */
  /* ====================================================================== */

"for"                   return tFOR;
"break"                 return tBREAK;
"continue"              return tCONTINUE;

  /* ====================================================================== */
  /* ====[                 6 - Expression Operators                   ]==== */
  /* ====================================================================== */

[\[\](),?]             return *yytext;
[-+*/%~<>@?=]          return *yytext;
"=="                   return tEQ;
"!="                   return tNE;
"<="                   return tLE;
">="                   return tGE;
"**"                   return tCONTRACT;
".capacity"            return tCAPACITY;
".dim"                 return tDIM;
".dims"                return tDIMS;
".rank"                return tRANK;
".reshape"             return tRESHAPE;
"input"                return tINPUT;

  /* ====================================================================== */
  /* ====[              3   - Delimiters and Separators               ]==== */
  /* ====[              4.3 - Function Body                           ]==== */
  /* ====[                 5.3 - Return Statement                     ]==== */
  /* ====================================================================== */

[#\[\](){};:!,]                return *yytext;
"return"                       return tRETURN;
"write"                        return tWRITE;
"writeln"                      return tWRITELN;

  /* ====================================================================== */
  /* ====[                   2.7.1 - Integers                         ]==== */
  /* ====================================================================== */

"0x"[[:xdigit:]]+        { yylval.i = strtoul(yytext + 2, NULL, 16); return tINTEGER; }
"0b"[01]+                { yylval.i = strtoul(yytext + 2, NULL, 2); return tINTEGER; }
"0"[0-7]+                { yylval.i = strtoul(yytext, NULL, 8); return tINTEGER; }
{INT}                    { yylval.i = strtoul(yytext, NULL, 10); return tINTEGER; }

  /* ====================================================================== */
  /* ====[                2.6 - Identifiers (names)                   ]==== */
  /* ====================================================================== */

{IDENTIFIER}             { yylval.s = new std::string(yytext); return tIDENTIFIER; }

  /* ====================================================================== */
  /* ====[              2.7.3 - Cadeias de caracteres                 ]==== */
  /* ====================================================================== */

\"                       { yy_push_state(X_STRING); strlit.str(""); }
<X_STRING>\"             { yy_pop_state(); yylval.s = new std::string(strlit.str()); return tSTRING; }
<X_STRING>\\\\           { strlit << '\\'; }
<X_STRING>\\\"           { strlit << '"'; }
<X_STRING>\\\'           { strlit << '\''; }
<X_STRING>\\n            { strlit << '\n'; }
<X_STRING>\\t            { strlit << '\t'; }
<X_STRING>\\[0-7]{1,3}   { 
                           int val = strtoul(yytext + 1, NULL, 8);
                           if (val > 255) yyerror("octal escape sequence out of range");
                           strlit << (char)val; 
                         }
<X_STRING>\\x[0-9a-fA-F]{1,2} { 
                           int val = strtoul(yytext + 2, NULL, 16);
                           if (val > 255) yyerror("hexadecimal escape sequence out of range");
                           strlit << (char)val; 
                         }
<X_STRING>\\.            { yyerror("unknown escape sequence"); }
<X_STRING>\n             { yyerror("newline in string"); }
<X_STRING>.              { strlit << yytext; }

[ \t\n]+                 ; /* ignore whitespace */

.                        { yyerror("Unknown character"); }

%%
