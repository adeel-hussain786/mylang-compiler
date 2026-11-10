#include "Parser.h"
#include <iostream>

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), current(0) {}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::TOKEN_EOF;
}

const Token& Parser::advance() {
    if (!isAtEnd()) {
        current++;
    }
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType t : types) {
        if (check(t)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& errorMessage) {
    if (check(type)) {
        return advance();
    }
    throw ParseError(errorMessage, peek().line, peek().column);
}

void Parser::recordError(const Token& token, const std::string& message) {
    errors.push_back("Syntax Error [line " + std::to_string(token.line) + 
                     ", col " + std::to_string(token.column) + "]: " + message);
}

void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::TOKEN_SEMICOLON) {
            return;
        }

        switch (peek().type) {
            case TokenType::TOKEN_KW_INT:
            case TokenType::TOKEN_KW_PRINT:
                return;
            default:
                break;
        }

        advance();
    }
}

std::unique_ptr<ProgramAST> Parser::parse() {
    auto program = std::make_unique<ProgramAST>();

    while (!isAtEnd()) {
        try {
            auto stmt = parseStatement();
            if (stmt) {
                program->addStatement(std::move(stmt));
            }
        } catch (const ParseError& e) {
            Token errToken(TokenType::TOKEN_UNKNOWN, "", e.line, e.column);
            recordError(errToken, e.what());
            synchronize();
        }
    }

    return program;
}

std::unique_ptr<StmtAST> Parser::parseStatement() {
    if (check(TokenType::TOKEN_KW_INT)) {
        return parseVarDecl();
    }
    if (check(TokenType::TOKEN_KW_PRINT)) {
        return parsePrint();
    }
    if (check(TokenType::TOKEN_IDENTIFIER)) {
        return parseAssignment();
    }

    throw ParseError("Expected statement (variable declaration, assignment, or print) but found '" + 
                     peek().lexeme + "'", peek().line, peek().column);
}

std::unique_ptr<VarDeclStmtAST> Parser::parseVarDecl() {
    const Token& typeToken = advance();
    const Token& nameToken = consume(TokenType::TOKEN_IDENTIFIER, "Expected variable name after 'int'.");

    std::unique_ptr<ExprAST> initializer = nullptr;
    if (match(TokenType::TOKEN_ASSIGN)) {
        initializer = parseExpression();
    }

    consume(TokenType::TOKEN_SEMICOLON, "Expected ';' after variable declaration.");
    return std::make_unique<VarDeclStmtAST>(typeToken.lexeme, nameToken.lexeme, std::move(initializer), typeToken.line, typeToken.column);
}

std::unique_ptr<AssignStmtAST> Parser::parseAssignment() {
    const Token& nameToken = advance();
    consume(TokenType::TOKEN_ASSIGN, "Expected '=' in assignment.");
    auto value = parseExpression();
    consume(TokenType::TOKEN_SEMICOLON, "Expected ';' after assignment.");
    return std::make_unique<AssignStmtAST>(nameToken.lexeme, std::move(value), nameToken.line, nameToken.column);
}

std::unique_ptr<PrintStmtAST> Parser::parsePrint() {
    const Token& printToken = advance();
    consume(TokenType::TOKEN_LPAREN, "Expected '(' after 'print'.");
    auto expr = parseExpression();
    consume(TokenType::TOKEN_RPAREN, "Expected ')' after print expression.");
    consume(TokenType::TOKEN_SEMICOLON, "Expected ';' after print statement.");
    return std::make_unique<PrintStmtAST>(std::move(expr), printToken.line, printToken.column);
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
    return parseComparison();
}

std::unique_ptr<ExprAST> Parser::parseComparison() {
    auto expr = parseAdditive();

    while (match({TokenType::TOKEN_EQUAL_EQUAL, TokenType::TOKEN_NOT_EQUAL, 
                  TokenType::TOKEN_LESS, TokenType::TOKEN_LESS_EQUAL, 
                  TokenType::TOKEN_GREATER, TokenType::TOKEN_GREATER_EQUAL})) {
        const Token& opToken = previous();
        auto rhs = parseAdditive();
        expr = std::make_unique<BinaryExprAST>(opToken.lexeme, std::move(expr), std::move(rhs), opToken.line, opToken.column);
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseAdditive() {
    auto expr = parseMultiplicative();

    while (match({TokenType::TOKEN_PLUS, TokenType::TOKEN_MINUS})) {
        const Token& opToken = previous();
        auto rhs = parseMultiplicative();
        expr = std::make_unique<BinaryExprAST>(opToken.lexeme, std::move(expr), std::move(rhs), opToken.line, opToken.column);
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseMultiplicative() {
    auto expr = parseUnary();

    while (match({TokenType::TOKEN_STAR, TokenType::TOKEN_SLASH})) {
        const Token& opToken = previous();
        auto rhs = parseUnary();
        expr = std::make_unique<BinaryExprAST>(opToken.lexeme, std::move(expr), std::move(rhs), opToken.line, opToken.column);
    }

    return expr;
}

std::unique_ptr<ExprAST> Parser::parseUnary() {
    if (match(TokenType::TOKEN_MINUS)) {
        const Token& opToken = previous();
        auto operand = parseUnary();
        return std::make_unique<UnaryExprAST>("-", std::move(operand), opToken.line, opToken.column);
    }
    if (match(TokenType::TOKEN_PLUS)) {
        return parseUnary();
    }
    return parsePrimary();
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
    if (match(TokenType::TOKEN_INT_LIT)) {
        int val = std::stoi(previous().lexeme);
        return std::make_unique<NumberExprAST>(val, previous().line, previous().column);
    }

    if (match(TokenType::TOKEN_IDENTIFIER)) {
        return std::make_unique<VariableExprAST>(previous().lexeme, previous().line, previous().column);
    }

    if (match(TokenType::TOKEN_LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::TOKEN_RPAREN, "Expected ')' after grouped expression.");
        return expr;
    }

    throw ParseError("Expected expression (number, variable, or '(') but found '" + 
                     peek().lexeme + "'", peek().line, peek().column);
}
