#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    TOKEN_EOF,
    TOKEN_UNKNOWN,

    TOKEN_KW_INT,
    TOKEN_KW_PRINT,

    TOKEN_INT_LIT,
    TOKEN_IDENTIFIER,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,

    TOKEN_EQUAL_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,

    TOKEN_ASSIGN,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON
};

std::string tokenTypeToString(TokenType type);

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    Token(TokenType type, std::string lexeme, int line, int column)
        : type(type), lexeme(std::move(lexeme)), line(line), column(column) {}
};

#endif // TOKEN_H
