#ifndef SCANNER_H
#define SCANNER_H

#include <iostream>
#include <string>
#include <memory>

// Токены
enum class TokenType {
    // Специальные символы
    EOF_TOKEN,
    ERROR_TOKEN,
    
    // Ключевые слова
    KEYWORD_IF,
    KEYWORD_ELSE,
    KEYWORD_WHILE,
    KEYWORD_FOR,
    KEYWORD_INT,
    KEYWORD_FLOAT,
    KEYWORD_BOOL,
    KEYWORD_VOID,
    KEYWORD_RETURN,
    KEYWORD_VAR,
    
    // Идентификаторы и литералы
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    
    // Операторы
    PLUS,           // +
    MINUS,          // -
    MULTIPLY,       // *
    DIVIDE,         // /
    ASSIGN,         // =
    EQ,             // ==
    NEQ,            // !=
    LT,             // <
    GT,             // >
    LE,             // <=
    GE,             // >=
    AND,            // &&
    OR,             // ||
    NOT,            // !
    
    // Разделители
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]
    SEMICOLON,      // ;
    COMMA,          // ,
    
    // Стрелка для функций (опционально)
    ARROW           // ->
};

// Структура токена
struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t = TokenType::ERROR_TOKEN, const std::string& v = "", int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
    
    std::string toString() const {
        std::string typeStr;
        switch (type) {
            case TokenType::EOF_TOKEN: typeStr = "EOF"; break;
            case TokenType::ERROR_TOKEN: typeStr = "ERROR"; break;
            case TokenType::KEYWORD_IF: typeStr = "IF"; break;
            case TokenType::KEYWORD_ELSE: typeStr = "ELSE"; break;
            case TokenType::KEYWORD_WHILE: typeStr = "WHILE"; break;
            case TokenType::KEYWORD_FOR: typeStr = "FOR"; break;
            case TokenType::KEYWORD_INT: typeStr = "INT"; break;
            case TokenType::KEYWORD_FLOAT: typeStr = "FLOAT"; break;
            case TokenType::KEYWORD_BOOL: typeStr = "BOOL"; break;
            case TokenType::KEYWORD_VOID: typeStr = "VOID"; break;
            case TokenType::KEYWORD_RETURN: typeStr = "RETURN"; break;
            case TokenType::KEYWORD_VAR: typeStr = "VAR"; break;
            case TokenType::IDENTIFIER: typeStr = "IDENTIFIER"; break;
            case TokenType::INT_LITERAL: typeStr = "INT_LITERAL"; break;
            case TokenType::FLOAT_LITERAL: typeStr = "FLOAT_LITERAL"; break;
            case TokenType::STRING_LITERAL: typeStr = "STRING_LITERAL"; break;
            case TokenType::PLUS: typeStr = "+"; break;
            case TokenType::MINUS: typeStr = "-"; break;
            case TokenType::MULTIPLY: typeStr = "*"; break;
            case TokenType::DIVIDE: typeStr = "/"; break;
            case TokenType::ASSIGN: typeStr = "="; break;
            case TokenType::EQ: typeStr = "=="; break;
            case TokenType::NEQ: typeStr = "!="; break;
            case TokenType::LT: typeStr = "<"; break;
            case TokenType::GT: typeStr = ">"; break;
            case TokenType::LE: typeStr = "<="; break;
            case TokenType::GE: typeStr = ">="; break;
            case TokenType::AND: typeStr = "&&"; break;
            case TokenType::OR: typeStr = "||"; break;
            case TokenType::NOT: typeStr = "!"; break;
            case TokenType::LPAREN: typeStr = "("; break;
            case TokenType::RPAREN: typeStr = ")"; break;
            case TokenType::LBRACE: typeStr = "{"; break;
            case TokenType::RBRACE: typeStr = "}"; break;
            case TokenType::LBRACKET: typeStr = "["; break;
            case TokenType::RBRACKET: typeStr = "]"; break;
            case TokenType::SEMICOLON: typeStr = ";"; break;
            case TokenType::COMMA: typeStr = ","; break;
            case TokenType::ARROW: typeStr = "->"; break;
        }
        return typeStr + (value.empty() ? "" : "('" + value + "')");
    }
};

// Класс лексического анализатора
class Scanner {
public:
    Scanner(std::istream& input = std::cin);
    
    // Получить следующий токен
    Token nextToken();
    
    // Пропустить пробелы и комментарии
    void skipWhitespaceAndComments();
    
    // Получить текущую позицию
    int getLine() const { return line; }
    int getColumn() const { return column; }
    
private:
    std::istream& input;
    int line;
    int column;
    int currentChar;
    
    // Считать следующий символ
    int advance();
    
    // Посмотреть следующий символ без перемещения
    int peek() const;
    
    // Распознать идентификатор или ключевое слово
    Token identifierOrKeyword();
    
    // Распознать число
    Token number();
    
    // Распознать строку
    Token string();
    
    // Распознать операторы
    Token operatorToken();
    
    // Проверить, является ли символ частью идентификатора
    bool isIdentifierStart(int c) const;
    
    // Проверить, является ли символ частью числа
    bool isDigit(int c) const;
    
    // Проверить, является ли символ частью идентификатора или числа
    bool isIdentifierChar(int c) const;
};

#endif // SCANNER_H
