%{
// Windows compatibility
#ifdef _WIN32
#define _CRT_NONSTDC_NO_DEPRECATE
#include <string.h>
#include <io.h>

#ifndef strdup
inline char* strdup(const char* s) {
    if (!s) return nullptr;
    size_t len = strlen(s) + 1;
    char* newstr = (char*)malloc(len);
    if (newstr) memcpy(newstr, s, len);
    return newstr;
}
#endif

#define isatty _isatty
#define fileno _fileno
#endif

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include "ast.h"
#include "parser.hpp"

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
"char"                  { return KEYWORD_CHAR; }
"void"                  { return KEYWORD_VOID; }
"return"                { return KEYWORD_RETURN; }
"true"                  { yylval.num = 1; return BOOL_LITERAL; }
"false"                 { yylval.num = 0; return BOOL_LITERAL; }

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
    char* str = (char*)malloc(strlen(yytext) - 1);
    strncpy(str, yytext + 1, strlen(yytext) - 2);
    str[strlen(yytext) - 2] = '\0';
    yylval.str = str;
    return STRING_LITERAL; 
}

\'.\'                   { 
    yylval.num = yytext[1]; 
    return CHAR_LITERAL; 
}

[a-zA-Z_][a-zA-Z0-9_]*  { 
    yylval.str = strdup(yytext); 
    return IDENTIFIER; 
}

[ \t\r\n]+              { /* skip whitespace */ }

"//".*                  { /* skip comments */ }

.                       { /* ignore unknown characters */ }

%%