#include "Lexer.h"
#include <cctype>

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::TOKEN_EOF:           return "TOKEN_EOF";
        case TokenType::TOKEN_UNKNOWN:       return "TOKEN_UNKNOWN";
        case TokenType::TOKEN_KW_INT:        return "TOKEN_KW_INT";
        case TokenType::TOKEN_KW_PRINT:      return "TOKEN_KW_PRINT";
        case TokenType::TOKEN_INT_LIT:       return "TOKEN_INT_LIT";
        case TokenType::TOKEN_IDENTIFIER:    return "TOKEN_IDENTIFIER";
        case TokenType::TOKEN_PLUS:          return "TOKEN_PLUS";
        case TokenType::TOKEN_MINUS:         return "TOKEN_MINUS";
        case TokenType::TOKEN_STAR:          return "TOKEN_STAR";
        case TokenType::TOKEN_SLASH:         return "TOKEN_SLASH";
        case TokenType::TOKEN_EQUAL_EQUAL:   return "TOKEN_EQUAL_EQUAL";
        case TokenType::TOKEN_NOT_EQUAL:     return "TOKEN_NOT_EQUAL";
        case TokenType::TOKEN_LESS:          return "TOKEN_LESS";
        case TokenType::TOKEN_LESS_EQUAL:    return "TOKEN_LESS_EQUAL";
        case TokenType::TOKEN_GREATER:       return "TOKEN_GREATER";
        case TokenType::TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TokenType::TOKEN_ASSIGN:        return "TOKEN_ASSIGN";
        case TokenType::TOKEN_LPAREN:        return "TOKEN_LPAREN";
        case TokenType::TOKEN_RPAREN:        return "TOKEN_RPAREN";
        case TokenType::TOKEN_SEMICOLON:     return "TOKEN_SEMICOLON";
        default:                             return "TOKEN_INVALID";
    }
}

Lexer::Lexer(std::string source)
    : source(std::move(source)), cursor(0), line(1), column(1) {}

bool Lexer::isAtEnd() const {
    return cursor >= source.length();
}

char Lexer::peek() const {
    return isAtEnd() ? '\0' : source[cursor];
}

char Lexer::peekNext() const {
    return (cursor + 1 >= source.length()) ? '\0' : source[cursor + 1];
}

char Lexer::advance() {
    char current = source[cursor++];
    column++;
    return current;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            cursor++;
            line++;
            column = 1;
        } else if (c == '/' && peekNext() == '/') {
            advance();
            advance();
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::scanNumber() {
    int startLine = line;
    int startCol = column;
    size_t startPos = cursor;

    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    std::string lexeme = source.substr(startPos, cursor - startPos);
    return Token(TokenType::TOKEN_INT_LIT, lexeme, startLine, startCol);
}

Token Lexer::scanIdentifierOrKeyword() {
    int startLine = line;
    int startCol = column;
    size_t startPos = cursor;

    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }

    std::string lexeme = source.substr(startPos, cursor - startPos);
    TokenType type = TokenType::TOKEN_IDENTIFIER;
    if (lexeme == "int") {
        type = TokenType::TOKEN_KW_INT;
    } else if (lexeme == "print") {
        type = TokenType::TOKEN_KW_PRINT;
    }

    return Token(type, lexeme, startLine, startCol);
}

Token Lexer::getNextToken() {
    skipWhitespaceAndComments();

    if (isAtEnd()) {
        return Token(TokenType::TOKEN_EOF, "", line, column);
    }

    int startLine = line;
    int startCol = column;
    char c = peek();

    if (std::isdigit(static_cast<unsigned char>(c))) {
        return scanNumber();
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return scanIdentifierOrKeyword();
    }

    advance();

    switch (c) {
        case '+': return Token(TokenType::TOKEN_PLUS, "+", startLine, startCol);
        case '-': return Token(TokenType::TOKEN_MINUS, "-", startLine, startCol);
        case '*': return Token(TokenType::TOKEN_STAR, "*", startLine, startCol);
        case '/': return Token(TokenType::TOKEN_SLASH, "/", startLine, startCol);
        case '(': return Token(TokenType::TOKEN_LPAREN, "(", startLine, startCol);
        case ')': return Token(TokenType::TOKEN_RPAREN, ")", startLine, startCol);
        case ';': return Token(TokenType::TOKEN_SEMICOLON, ";", startLine, startCol);

        case '=':
            if (peek() == '=') {
                advance();
                return Token(TokenType::TOKEN_EQUAL_EQUAL, "==", startLine, startCol);
            }
            return Token(TokenType::TOKEN_ASSIGN, "=", startLine, startCol);

        case '!':
            if (peek() == '=') {
                advance();
                return Token(TokenType::TOKEN_NOT_EQUAL, "!=", startLine, startCol);
            }
            return Token(TokenType::TOKEN_UNKNOWN, "!", startLine, startCol);

        case '<':
            if (peek() == '=') {
                advance();
                return Token(TokenType::TOKEN_LESS_EQUAL, "<=", startLine, startCol);
            }
            return Token(TokenType::TOKEN_LESS, "<", startLine, startCol);

        case '>':
            if (peek() == '=') {
                advance();
                return Token(TokenType::TOKEN_GREATER_EQUAL, ">=", startLine, startCol);
            }
            return Token(TokenType::TOKEN_GREATER, ">", startLine, startCol);

        default:
            return Token(TokenType::TOKEN_UNKNOWN, std::string(1, c), startLine, startCol);
    }
}

std::vector<Token> Lexer::tokenizeAll() {
    std::vector<Token> tokens;
    while (true) {
        Token token = getNextToken();
        tokens.push_back(token);
        if (token.type == TokenType::TOKEN_EOF) {
            break;
        }
    }
    return tokens;
}
