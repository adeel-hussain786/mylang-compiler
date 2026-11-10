#ifndef PARSER_H
#define PARSER_H

#include "lexer/Token.h"
#include "ast/AST.h"
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <initializer_list>

class ParseError : public std::runtime_error {
public:
    int line;
    int column;

    ParseError(const std::string& message, int line, int column)
        : std::runtime_error(message), line(line), column(column) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::unique_ptr<ProgramAST> parse();
    bool hasErrors() const { return !errors.empty(); }
    const std::vector<std::string>& getErrors() const { return errors; }

private:
    std::vector<Token> tokens;
    size_t current;
    std::vector<std::string> errors;

    const Token& peek() const;
    const Token& previous() const;
    bool isAtEnd() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    const Token& consume(TokenType type, const std::string& errorMessage);

    void recordError(const Token& token, const std::string& message);
    void synchronize();

    std::unique_ptr<StmtAST> parseStatement();
    std::unique_ptr<VarDeclStmtAST> parseVarDecl();
    std::unique_ptr<AssignStmtAST> parseAssignment();
    std::unique_ptr<PrintStmtAST> parsePrint();

    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseComparison();
    std::unique_ptr<ExprAST> parseAdditive();
    std::unique_ptr<ExprAST> parseMultiplicative();
    std::unique_ptr<ExprAST> parseUnary();
    std::unique_ptr<ExprAST> parsePrimary();
};

#endif // PARSER_H
