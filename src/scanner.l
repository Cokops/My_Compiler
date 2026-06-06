%{
#include <iostream>
#include <string>
#include <memory>
#include "parser.hpp"
#include "ast.h"

// Глобальные переменные
extern Scanner* g_scanner;
extern ProgramAST* g_program;

#define YY_DECL int yylex()
%}

%option noyywrap

%option case-insensitive

%{
// Прототипы для создания AST узлов
extern std::unique_ptr<ExprAST> make_number_expr(double value);
extern std::unique_ptr<ExprAST> make_string_expr(const std::string& value);
extern std::unique_ptr<ExprAST> make_variable_expr(const std::string& name);
%}

%%

"if"          { return KEYWORD_IF; }
"else"        { return KEYWORD_ELSE; }
"while"       { return KEYWORD_WHILE; }
"for"         { return KEYWORD_FOR; }
"int"         { return KEYWORD_INT; }
"float"       { return KEYWORD_FLOAT; }
"bool"        { return KEYWORD_BOOL; }
"void"        { return KEYWORD_VOID; }
"return"      { return KEYWORD_RETURN; }
"var"         { return KEYWORD_VAR; }

"=="          { return EQ; }
"!="          { return NEQ; }
"<="          { return LE; }
">="          { return GE; }
"&&"          { return AND; }
"||"          { return OR; }
"->"          { return ARROW; }

"+"           { return PLUS; }
"-"           { return MINUS; }
"*"           { return MULTIPLY; }
"/"           { return DIVIDE; }
"="           { return ASSIGN; }
"!"           { return NOT; }

"<"           { return LT; }
">"           { return GT; }

"("           { return LPAREN; }
")"           { return RPAREN; }
"{"           { return LBRACE; }
"}"           { return RBRACE; }
"["           { return LBRACKET; }
"]"           { return RBRACKET; }
";"           { return SEMICOLON; }
","           { return COMMA; }

[0-9]+(\\.[0-9]+)?  {
    yylval.expr = std::make_unique<NumberExprAST>(std::stod(yytext));
    return INT_LITERAL;
}

\"([^\"\\\\]|\\\\.)*\"  {
    std::string str(yytext + 1, yytext + strlen(yytext) - 1);
    // Обработка escape-последовательностей
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            i++;
            if (str[i] == 'n') result += '\n';
            else if (str[i] == 't') result += '\t';
            else if (str[i] == '"') result += '"';
            else if (str[i] == '\\') result += '\\';
            else result += str[i];
        } else {
            result += str[i];
        }
    }
    yylval.expr = std::make_unique<StringExprAST>(result);
    return STRING_LITERAL;
}

[a-zA-Z_][a-zA-Z0-9_]*  {
    yylval.expr = std::make_unique<VariableExprAST>(yytext);
    return IDENTIFIER;
}

[ \t\r\n]+    { /* пропустить пробелы */ }

.             { /* пропустить неизвестные символы */ }

%%
