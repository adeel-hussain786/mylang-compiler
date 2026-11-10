#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "ast/AST.h"
#include <string>
#include <vector>
#include <unordered_map>

struct Symbol {
    std::string name;
    std::string typeName;
    int line;
    int column;
    bool isInitialized;
};

class SymbolTable {
public:
    bool declare(const std::string& name, const std::string& typeName, int line, int column, bool isInitialized);
    bool isDeclared(const std::string& name) const;
    const Symbol* lookup(const std::string& name) const;
    void markInitialized(const std::string& name);
    const std::unordered_map<std::string, Symbol>& getAllSymbols() const { return symbols; }

private:
    std::unordered_map<std::string, Symbol> symbols;
};

class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer() = default;

    bool analyze(const ProgramAST& program);
    bool hasErrors() const { return !errors.empty(); }
    const std::vector<std::string>& getErrors() const { return errors; }
    const SymbolTable& getSymbolTable() const { return symbolTable; }

    void visit(const NumberExprAST& node) override;
    void visit(const VariableExprAST& node) override;
    void visit(const BinaryExprAST& node) override;
    void visit(const UnaryExprAST& node) override;
    void visit(const VarDeclStmtAST& node) override;
    void visit(const AssignStmtAST& node) override;
    void visit(const PrintStmtAST& node) override;
    void visit(const ProgramAST& node) override;

private:
    SymbolTable symbolTable;
    std::vector<std::string> errors;

    void recordError(int line, int column, const std::string& message);
};

#endif // SEMANTIC_ANALYZER_H
