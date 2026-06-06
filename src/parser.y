%{
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include "ast.h"
#include "scanner.h"

// Прототипы функций
void yyerror(const char* s);
int yylex();

// Глобальные переменные
extern Scanner* g_scanner;
extern ProgramAST* g_program;
%}

%define api.token.constructor
%define api.value.type {std::unique_ptr<ASTNode>}
%define parse.error verbose

%union {
    std::unique_ptr<ExprAST> expr;
    std::unique_ptr<ASTNode> stmt;
    std::unique_ptr<VarDeclAST> varDecl;
    std::vector<std::unique_ptr<ASTNode>>* stmtList;
    std::vector<std::pair<std::string, std::string>>* paramList;
    std::string* str;
}

// Токены с типами
%token EOF_TOKEN ERROR_TOKEN
%token KEYWORD_IF KEYWORD_ELSE KEYWORD_WHILE KEYWORD_FOR
%token KEYWORD_INT KEYWORD_FLOAT KEYWORD_BOOL KEYWORD_VOID KEYWORD_RETURN
%token IDENTIFIER INT_LITERAL FLOAT_LITERAL STRING_LITERAL
%token PLUS MINUS MULTIPLY DIVIDE ASSIGN EQ NEQ LT GT LE GE AND OR NOT
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET SEMICOLON COMMA

// Типы для правил грамматики
%type <expr> expression assignmentExpr logicalOrExpr logicalAndExpr equalityExpr
%type <expr> relationalExpr additiveExpr multiplicativeExpr unaryExpr primaryExpr
%type <expr> functionCall numberExpr stringExpr
%type <stmt> statement ifStmt whileStmt forStmt returnStmt blockStmt
%type <varDecl> variableDecl
%type <paramList> parameterList
%type <stmtList> statementList
%type <str> type

%start program

%%

program
    : functionDefinition program
        { 
            if (g_program && $1) {
                auto func = dynamic_cast<FunctionAST*>($1.release());
                if (func) {
                    g_program->addFunction(std::unique_ptr<FunctionAST>(func));
                }
            }
            $$ = nullptr;
        }
    | /* пусто */
        { $$ = nullptr; }
    ;

functionDefinition
    : type IDENTIFIER LPAREN parameterList RPAREN blockStmt
        { 
            std::string typeStr = $1 ? *$1 : "int";
            if ($1) delete $1;
            $$ = std::make_unique<FunctionAST>($2->value, typeStr, *$4, std::move($6)); 
            delete $4;
        }
    ;

parameterList
    : parameter
        { 
            $$ = new std::vector<std::pair<std::string, std::string>>();
            $$->push_back(std::make_pair($1.name, $1.type));
        }
    | parameter COMMA parameterList
        { 
            $$ = $3;
            $$->push_back(std::make_pair($1.name, $1.type));
        }
    | /* пусто */
        { $$ = new std::vector<std::pair<std::string, std::string>>(); }
    ;

parameter
    : type IDENTIFIER
        { 
            $$.name = $2->value;
            $$.type = $1 ? *$1 : "int";
        }
    ;

type
    : KEYWORD_INT { $$ = new std::string("int"); }
    | KEYWORD_FLOAT { $$ = new std::string("float"); }
    | KEYWORD_BOOL { $$ = new std::string("bool"); }
    | KEYWORD_VOID { $$ = new std::string("void"); }
    ;

blockStmt
    : LBRACE statementList RBRACE
        { 
            auto block = std::make_unique<BlockStmtAST>();
            if ($2) {
                for (auto& stmt : *$2) {
                    block->addStatement(std::move(stmt));
                }
                delete $2;
            }
            $$ = std::move(block);
        }
    ;

statementList
    : statement statementList
        { 
            if ($2) {
                $2->insert($2->begin(), std::move($1));
            } else {
                $2 = new std::vector<std::unique_ptr<ASTNode>>();
                $2->push_back(std::move($1));
            }
            $$ = $2;
        }
    | statement
        { 
            $$ = new std::vector<std::unique_ptr<ASTNode>>();
            $$->push_back(std::move($1));
        }
    ;

statement
    : variableDecl SEMICOLON
        { $$ = std::move($1); }
    | assignmentExpr SEMICOLON
        { $$ = std::move($1); }
    | ifStmt
        { $$ = std::move($1); }
    | whileStmt
        { $$ = std::move($1); }
    | forStmt
        { $$ = std::move($1); }
    | returnStmt SEMICOLON
        { $$ = std::move($1); }
    | blockStmt
        { $$ = std::move($1); }
    | SEMICOLON
        { 
            $$ = std::make_unique<BlockStmtAST>();
        }
    ;

variableDecl
    : type IDENTIFIER
        { 
            std::string typeStr = $1 ? *$1 : "int";
            if ($1) delete $1;
            $$ = std::make_unique<VarDeclAST>($2->value, typeStr);
        }
    | type IDENTIFIER ASSIGN expression
        { 
            std::string typeStr = $1 ? *$1 : "int";
            if ($1) delete $1;
            $$ = std::make_unique<VarDeclAST>($2->value, typeStr, std::move($4));
        }
    ;

assignmentExpr
    : IDENTIFIER ASSIGN expression
        { $$ = std::make_unique<AssignExprAST>($1->value, std::move($3)); }
    ;

ifStmt
    : KEYWORD_IF LPAREN expression RPAREN statement
        { 
            $$ = std::make_unique<IfStmtAST>(std::move($3), std::move($5)); 
        }
    | KEYWORD_IF LPAREN expression RPAREN statement KEYWORD_ELSE statement
        { 
            $$ = std::make_unique<IfStmtAST>(std::move($3), std::move($5), std::move($7)); 
        }
    ;

whileStmt
    : KEYWORD_WHILE LPAREN expression RPAREN statement
        { 
            $$ = std::make_unique<WhileStmtAST>(std::move($3), std::move($5)); 
        }
    ;

forStmt
    : KEYWORD_FOR LPAREN variableDecl SEMICOLON expression SEMICOLON expression RPAREN statement
        { 
            $$ = std::make_unique<ForStmtAST>(std::move($3), std::move($5), std::move($7), std::move($9)); 
        }
    | KEYWORD_FOR LPAREN expression SEMICOLON expression SEMICOLON expression RPAREN statement
        { 
            // Для простоты пропускаем сложную обработку первого выражения
            $$ = std::make_unique<ForStmtAST>(nullptr, std::move($5), std::move($7), std::move($9)); 
        }
    ;

returnStmt
    : KEYWORD_RETURN expression
        { $$ = std::make_unique<ReturnStmtAST>(std::move($2)); }
    | KEYWORD_RETURN
        { $$ = std::make_unique<ReturnStmtAST>(nullptr); }
    ;

expression
    : logicalOrExpr
        { $$ = std::move($1); }
    ;

logicalOrExpr
    : logicalAndExpr
        { $$ = std::move($1); }
    | logicalOrExpr OR logicalAndExpr
        { $$ = std::make_unique<BinaryExprAST>('|', std::move($1), std::move($3)); }
    ;

logicalAndExpr
    : equalityExpr
        { $$ = std::move($1); }
    | logicalAndExpr AND equalityExpr
        { $$ = std::make_unique<BinaryExprAST>('&', std::move($1), std::move($3)); }
    ;

equalityExpr
    : relationalExpr
        { $$ = std::move($1); }
    | equalityExpr EQ relationalExpr
        { $$ = std::make_unique<BinaryExprAST>('=', std::move($1), std::move($3)); }
    | equalityExpr NEQ relationalExpr
        { 
            auto eqExpr = std::make_unique<BinaryExprAST>('=', std::move($1), std::move($3));
            auto notOp = std::make_unique<BinaryExprAST>('!', std::move(eqExpr), nullptr); 
            $$ = std::move(notOp);
        }
    ;

relationalExpr
    : additiveExpr
        { $$ = std::move($1); }
    | relationalExpr LT additiveExpr
        { $$ = std::make_unique<BinaryExprAST>('<', std::move($1), std::move($3)); }
    | relationalExpr GT additiveExpr
        { $$ = std::make_unique<BinaryExprAST>('>', std::move($1), std::move($3)); }
    | relationalExpr LE additiveExpr
        { 
            auto gtExpr = std::make_unique<BinaryExprAST>('>', std::move($1), std::move($3));
            auto notOp = std::make_unique<BinaryExprAST>('!', std::move(gtExpr), nullptr); 
            $$ = std::move(notOp);
        }
    | relationalExpr GE additiveExpr
        { 
            auto ltExpr = std::make_unique<BinaryExprAST>('<', std::move($1), std::move($3));
            auto notOp = std::make_unique<BinaryExprAST>('!', std::move(ltExpr), nullptr); 
            $$ = std::move(notOp);
        }
    ;

additiveExpr
    : multiplicativeExpr
        { $$ = std::move($1); }
    | additiveExpr PLUS multiplicativeExpr
        { $$ = std::make_unique<BinaryExprAST>('+', std::move($1), std::move($3)); }
    | additiveExpr MINUS multiplicativeExpr
        { $$ = std::make_unique<BinaryExprAST>('-', std::move($1), std::move($3)); }
    ;

multiplicativeExpr
    : unaryExpr
        { $$ = std::move($1); }
    | multiplicativeExpr MULTIPLY unaryExpr
        { $$ = std::make_unique<BinaryExprAST>('*', std::move($1), std::move($3)); }
    | multiplicativeExpr DIVIDE unaryExpr
        { $$ = std::make_unique<BinaryExprAST>('/', std::move($1), std::move($3)); }
    ;

unaryExpr
    : primaryExpr
        { $$ = std::move($1); }
    | PLUS unaryExpr
        { $$ = std::move($2); }
    | MINUS unaryExpr
        { 
            auto zero = std::make_unique<NumberExprAST>(0.0);
            $$ = std::make_unique<BinaryExprAST>('-', std::move(zero), std::move($2)); 
        }
    | NOT unaryExpr
        { 
            auto zero = std::make_unique<NumberExprAST>(0.0);
            $$ = std::make_unique<BinaryExprAST>('=', std::move(zero), std::move($2)); 
        }
    ;

primaryExpr
    : IDENTIFIER
        { $$ = std::make_unique<VariableExprAST>($1->value); }
    | numberExpr
        { $$ = std::move($1); }
    | stringExpr
        { $$ = std::move($1); }
    | LPAREN expression RPAREN
        { $$ = std::move($2); }
    | functionCall
        { $$ = std::move($1); }
    ;

numberExpr
    : INT_LITERAL
        { $$ = std::make_unique<NumberExprAST>(std::stod($1->value)); }
    | FLOAT_LITERAL
        { $$ = std::make_unique<NumberExprAST>(std::stod($1->value)); }
    ;

stringExpr
    : STRING_LITERAL
        { $$ = std::make_unique<StringExprAST>($1->value); }
    ;

functionCall
    : IDENTIFIER LPAREN argumentList RPAREN
        { 
            $$ = std::make_unique<CallExprAST>($1->value, std::move($3)); 
        }
    ;

argumentList
    : expression
        { 
            $$ = std::vector<std::unique_ptr<ExprAST>>();
            $$->push_back(std::move($1)); 
        }
    | expression COMMA argumentList
        { 
            $$ = std::move($3);
            $$->insert($$->begin(), std::move($1));
        }
    | /* пусто */
        { $$ = std::vector<std::unique_ptr<ExprAST>>(); }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Parse error at line " << g_scanner->getLine() 
              << ", column " << g_scanner->getColumn() << ": " << s << std::endl;
}
