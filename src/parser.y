%{
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cstring>
#include <cstdlib>
#include "ast.h"

#ifdef _WIN32
#include <string.h>
inline char* strdup(const char* s) {
    if (!s) return nullptr;
    size_t len = strlen(s) + 1;
    char* newstr = (char*)malloc(len);
    if (newstr) memcpy(newstr, s, len);
    return newstr;
}
#endif

void yyerror(const char* s);
int yylex();

extern ProgramAST* g_program;
%}

%union {
    double num;
    char* str;
    ExprAST* expr;
    ASTNode* stmt;
    VarDeclAST* varDecl;
    FunctionAST* func;
    std::vector<ASTNode*>* stmtList;
    std::vector<std::pair<std::string, std::string>>* paramList;
    std::vector<ExprAST*>* argList;
    std::pair<std::string, std::string>* param;
}

%token <num> INT_LITERAL FLOAT_LITERAL BOOL_LITERAL
%token <str> IDENTIFIER STRING_LITERAL

%token EOF_TOKEN 0 "end of file"
%token ERROR_TOKEN
%token KEYWORD_IF KEYWORD_ELSE KEYWORD_WHILE KEYWORD_FOR
%token KEYWORD_INT KEYWORD_FLOAT KEYWORD_BOOL KEYWORD_VOID KEYWORD_RETURN
%token PLUS MINUS MULTIPLY DIVIDE ASSIGN EQ NEQ LT GT LE GE AND OR NOT
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET SEMICOLON COMMA

%right ASSIGN
%left OR
%left AND
%left EQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left MULTIPLY DIVIDE
%right NOT

%type <func> functionDefinition
%type <expr> expression assignmentExpr logicalOrExpr logicalAndExpr equalityExpr
%type <expr> relationalExpr additiveExpr multiplicativeExpr unaryExpr primaryExpr
%type <expr> functionCall numberExpr stringExpr boolExpr
%type <stmt> statement ifStmt whileStmt forStmt returnStmt blockStmt
%type <varDecl> variableDecl
%type <paramList> parameterList
%type <stmtList> statementList
%type <argList> argumentList
%type <str> type
%type <param> parameter

%start program

%%

program
    : functionDefinition program
        { 
            if (g_program && $1) {
                g_program->addFunction($1);
            }
        }
    | /* empty */
        { }
    ;

functionDefinition
    : type IDENTIFIER LPAREN parameterList RPAREN blockStmt
        { 
            std::string typeStr = ($1 != nullptr) ? $1 : "int";
            $$ = new FunctionAST($2, typeStr, *$4, $6);
            delete $4;
            if ($1) free($1);
            if ($2) free($2);
        }
    ;

parameterList
    : parameter
        { 
            $$ = new std::vector<std::pair<std::string, std::string>>();
            $$->push_back(*$1);
            delete $1;
        }
    | parameter COMMA parameterList
        { 
            $$ = $3;
            $$->insert($$->begin(), *$1);
            delete $1;
        }
    | /* empty */
        { $$ = new std::vector<std::pair<std::string, std::string>>(); }
    ;

parameter
    : type IDENTIFIER
        { 
            $$ = new std::pair<std::string, std::string>(
                std::string($2),
                std::string(($1 != nullptr) ? $1 : "int")
            );
            if ($1) free($1);
            if ($2) free($2);
        }
    ;

type
    : KEYWORD_INT { $$ = strdup("int"); }
    | KEYWORD_FLOAT { $$ = strdup("float"); }
    | KEYWORD_BOOL { $$ = strdup("bool"); }
    | KEYWORD_VOID { $$ = strdup("void"); }
    ;

blockStmt
    : LBRACE statementList RBRACE
        { 
            auto* block = new BlockStmtAST();
            if ($2) {
                for (auto* stmt : *$2) {
                    block->addStatement(stmt);
                }
                delete $2;
            }
            $$ = block;
        }
    ;

statementList
    : statement statementList
        { 
            if ($2) {
                $2->insert($2->begin(), $1);
                $$ = $2;
            } else {
                $$ = new std::vector<ASTNode*>();
                $$->push_back($1);
            }
        }
    | statement
        { 
            $$ = new std::vector<ASTNode*>();
            $$->push_back($1);
        }
    ;

statement
    : variableDecl SEMICOLON
        { $$ = $1; }
    | assignmentExpr SEMICOLON
        { $$ = $1; }
    | ifStmt
        { $$ = $1; }
    | whileStmt
        { $$ = $1; }
    | forStmt
        { $$ = $1; }
    | returnStmt SEMICOLON
        { $$ = $1; }
    | blockStmt
        { $$ = $1; }
    | SEMICOLON
        { 
            $$ = new BlockStmtAST();
        }
    ;

variableDecl
    : type IDENTIFIER
        { 
            std::string typeStr = ($1 != nullptr) ? $1 : "int";
            $$ = new VarDeclAST($2, typeStr);
            if ($1) free($1);
            if ($2) free($2);
        }
    | type IDENTIFIER ASSIGN expression
        { 
            std::string typeStr = ($1 != nullptr) ? $1 : "int";
            $$ = new VarDeclAST($2, typeStr, $4);
            if ($1) free($1);
            if ($2) free($2);
        }
    ;

assignmentExpr
    : IDENTIFIER ASSIGN expression
        { 
            $$ = new AssignExprAST($1, $3);
            if ($1) free($1);
        }
    ;

ifStmt
    : KEYWORD_IF LPAREN expression RPAREN statement
        { 
            $$ = new IfStmtAST($3, $5); 
        }
    | KEYWORD_IF LPAREN expression RPAREN statement KEYWORD_ELSE statement
        { 
            $$ = new IfStmtAST($3, $5, $7); 
        }
    ;

whileStmt
    : KEYWORD_WHILE LPAREN expression RPAREN statement
        { 
            $$ = new WhileStmtAST($3, $5); 
        }
    ;

forStmt
    : KEYWORD_FOR LPAREN variableDecl SEMICOLON expression SEMICOLON expression RPAREN statement
        { 
            $$ = new ForStmtAST($3, $5, $7, $9); 
        }
    ;

returnStmt
    : KEYWORD_RETURN expression
        { $$ = new ReturnStmtAST($2); }
    | KEYWORD_RETURN
        { $$ = new ReturnStmtAST(nullptr); }
    ;

expression
    : logicalOrExpr
        { $$ = $1; }
    ;

logicalOrExpr
    : logicalAndExpr
        { $$ = $1; }
    | logicalOrExpr OR logicalAndExpr
        { $$ = new BinaryExprAST(OR, $1, $3); }
    ;

logicalAndExpr
    : equalityExpr
        { $$ = $1; }
    | logicalAndExpr AND equalityExpr
        { $$ = new BinaryExprAST(AND, $1, $3); }
    ;

equalityExpr
    : relationalExpr
        { $$ = $1; }
    | equalityExpr EQ relationalExpr
        { $$ = new BinaryExprAST(EQ, $1, $3); }
    | equalityExpr NEQ relationalExpr
        { 
            auto* eqExpr = new BinaryExprAST(EQ, $1, $3);
            $$ = new BinaryExprAST(NOT, eqExpr, nullptr);
        }
    ;

relationalExpr
    : additiveExpr
        { $$ = $1; }
    | relationalExpr LT additiveExpr
        { $$ = new BinaryExprAST(LT, $1, $3); }
    | relationalExpr GT additiveExpr
        { $$ = new BinaryExprAST(GT, $1, $3); }
    | relationalExpr LE additiveExpr
        { $$ = new BinaryExprAST(LE, $1, $3); }
    | relationalExpr GE additiveExpr
        { $$ = new BinaryExprAST(GE, $1, $3); }
    ;

additiveExpr
    : multiplicativeExpr
        { $$ = $1; }
    | additiveExpr PLUS multiplicativeExpr
        { $$ = new BinaryExprAST(PLUS, $1, $3); }
    | additiveExpr MINUS multiplicativeExpr
        { $$ = new BinaryExprAST(MINUS, $1, $3); }
    ;

multiplicativeExpr
    : unaryExpr
        { $$ = $1; }
    | multiplicativeExpr MULTIPLY unaryExpr
        { $$ = new BinaryExprAST(MULTIPLY, $1, $3); }
    | multiplicativeExpr DIVIDE unaryExpr
        { $$ = new BinaryExprAST(DIVIDE, $1, $3); }
    ;

unaryExpr
    : primaryExpr
        { $$ = $1; }
    | PLUS unaryExpr
        { $$ = $2; }
    | MINUS unaryExpr
        { 
            auto* zero = new NumberExprAST(0.0);
            $$ = new BinaryExprAST(MINUS, zero, $2);
        }
    | NOT unaryExpr
        { 
            auto* zero = new NumberExprAST(0.0);
            $$ = new BinaryExprAST(EQ, zero, $2);
        }
    ;

primaryExpr
    : IDENTIFIER
        { 
            $$ = new VariableExprAST($1);
            if ($1) free($1);
        }
    | numberExpr
        { $$ = $1; }
    | boolExpr
        { $$ = $1; }
    | stringExpr
        { $$ = $1; }
    | LPAREN expression RPAREN
        { $$ = $2; }
    | functionCall
        { $$ = $1; }
    ;

numberExpr
    : INT_LITERAL
        { $$ = new NumberExprAST($1); }
    | FLOAT_LITERAL
        { $$ = new NumberExprAST($1, true); }
    ;

boolExpr
    : BOOL_LITERAL
        { $$ = new BoolExprAST($1 != 0); }
    ;

stringExpr
    : STRING_LITERAL
        { 
            $$ = new StringExprAST($1);
            if ($1) free($1);
        }
    ;

functionCall
    : IDENTIFIER LPAREN argumentList RPAREN
        { 
            $$ = new CallExprAST($1, *$3);
            delete $3;
            if ($1) free($1);
        }
    ;

argumentList
    : expression
        { 
            $$ = new std::vector<ExprAST*>();
            $$->push_back($1);
        }
    | expression COMMA argumentList
        { 
            $$ = $3;
            $$->insert($$->begin(), $1);
        }
    | /* empty */
        { $$ = new std::vector<ExprAST*>(); }
    ;

%%

void yyerror(const char* s) {
    extern int yylineno;
    std::cerr << "Parse error at line " << yylineno << ": " << s << std::endl;
}