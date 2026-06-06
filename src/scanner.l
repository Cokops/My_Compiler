%{
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include "parser.hpp"
#include "ast.h"
#include "scanner.h"

// Windows compatibility
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#define strdup _strdup
#endif

extern Scanner* g_scanner;
extern ProgramAST* g_program;
%}

%option noyywrap
%option yylineno
%option nounistd

%%

"if"                    { return KEYWORD_IF; }
"else"                  { return KEYWORD_ELSE; }
"while"                 { return KEYWORD_WHILE; }
"for"                   { return KEYWORD_FOR; }
"int"                   { return KEYWORD_INT; }
"float"                 { return KEYWORD_FLOAT; }
"bool"                  { return KEYWORD_BOOL; }
"void"                  { return KEYWORD_VOID; }
"return"                { return KEYWORD_RETURN; }

"=="                    { return EQ; }
"!="                    { return NEQ; }
"<="                    { return LE; }
">="                    { return GE; }
"&&"                    { return AND; }
"||"                    { return OR; }

"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MULTIPLY; }
"/"                     { return DIVIDE; }
"="                     { return ASSIGN; }
"!"                     { return NOT; }

"<"                     { return LT; }
">"                     { return GT; }

"("                     { return LPAREN; }
")"                     { return RPAREN; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
"["                     { return LBRACKET; }
"]"                     { return RBRACKET; }
";"                     { return SEMICOLON; }
","                     { return COMMA; }

[0-9]+                  { 
    yylval.num = atoi(yytext); 
    return INT_LITERAL; 
}

[0-9]+"."[0-9]+         { 
    yylval.num = atof(yytext); 
    return FLOAT_LITERAL; 
}

\"[^\"]*\"              { 
    yylval.str = _strdup(yytext); 
    return STRING_LITERAL; 
}

[a-zA-Z_][a-zA-Z0-9_]*  { 
    yylval.str = _strdup(yytext); 
    return IDENTIFIER; 
}

[ \t\r\n]+              { /* пропустить пробелы */ }

"//".*                  { /* пропустить комментарии */ }

.                       { /* игнорировать неизвестные символы */ }

%%