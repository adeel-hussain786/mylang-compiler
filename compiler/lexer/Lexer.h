#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::string source);

    Token getNextToken();
    std::vector<Token> tokenizeAll();

private:
    std::string source;
    size_t cursor;
    int line;
    int column;

    char peek() const;
    char peekNext() const;
    char advance();
    bool isAtEnd() const;

    void skipWhitespaceAndComments();
    Token scanNumber();
    Token scanIdentifierOrKeyword();
};

#endif // LEXER_H
