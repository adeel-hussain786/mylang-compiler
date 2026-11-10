#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

class ASTVisitor;
class NumberExprAST;
class VariableExprAST;
class BinaryExprAST;
class UnaryExprAST;
class VarDeclStmtAST;
class AssignStmtAST;
class PrintStmtAST;
class ProgramAST;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(const NumberExprAST& node) = 0;
    virtual void visit(const VariableExprAST& node) = 0;
    virtual void visit(const BinaryExprAST& node) = 0;
    virtual void visit(const UnaryExprAST& node) = 0;
    virtual void visit(const VarDeclStmtAST& node) = 0;
    virtual void visit(const AssignStmtAST& node) = 0;
    virtual void visit(const PrintStmtAST& node) = 0;
    virtual void visit(const ProgramAST& node) = 0;
};

class ASTNode {
public:
    int line;
    int column;

    ASTNode(int line, int column) : line(line), column(column) {}
    virtual ~ASTNode() = default;

    virtual void accept(ASTVisitor& visitor) const = 0;
    virtual void print(int indent = 0) const = 0;
};

class ExprAST : public ASTNode {
public:
    ExprAST(int line, int column) : ASTNode(line, column) {}
};

class StmtAST : public ASTNode {
public:
    StmtAST(int line, int column) : ASTNode(line, column) {}
};

class NumberExprAST : public ExprAST {
public:
    int value;

    NumberExprAST(int value, int line, int column)
        : ExprAST(line, column), value(value) {}

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    void print(int indent = 0) const override;
};

class VariableExprAST : public ExprAST {
public:
    std::string name;

    VariableExprAST(std::string name, int line, int column)
        : ExprAST(line, column), name(std::move(name)) {}

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    void print(int indent = 0) const override;
};

class BinaryExprAST : public ExprAST {
public:
    std::string op;
    std::unique_ptr<ExprAST> lhs;
    std::unique_ptr<ExprAST> rhs;

    BinaryExprAST(std::string op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs, int line, int column)
        : ExprAST(line, column), op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    void print(int indent = 0) const override;
};

class UnaryExprAST : public ExprAST {
public:
    std::string op;
    std::unique_ptr<ExprAST> operand;

    UnaryExprAST(std::string op, std::unique_ptr<ExprAST> operand, int line, int column)
        : ExprAST(line, column), op(std::move(op)), operand(std::move(operand)) {}

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    void print(int indent = 0) const override;
};

class VarDeclStmtAST : public StmtAST {
public:
    std::string typeName;
    std::string varName;
    std::unique_ptr<ExprAST> initializer;

    VarDeclStmtAST(std::string typeName, std::string varName, std::unique_ptr<ExprAST> initializer, int line, int column)
        : StmtAST(line, column), typeName(std::move(typeName)), varName(std::move(varName)), initializer(std::move(initializer)) {}

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    void print(int indent = 0) const override;
};

class AssignStmtAST : public StmtAST {
public:
    std::string varName;
    std::unique_ptr<ExprAST> value;

    AssignStmtAST(std::string varName, std::unique_ptr<ExprAST> value, int line, int column)
        : StmtAST(line, column), varName(std::move(varName)), value(std::move(value)) {}

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    void print(int indent = 0) const override;
};

class PrintStmtAST : public StmtAST {
public:
    std::unique_ptr<ExprAST> expr;

    PrintStmtAST(std::unique_ptr<ExprAST> expr, int line, int column)
        : StmtAST(line, column), expr(std::move(expr)) {}

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    void print(int indent = 0) const override;
};

class ProgramAST : public ASTNode {
public:
    std::vector<std::unique_ptr<StmtAST>> statements;

    ProgramAST() : ASTNode(1, 1) {}

    void addStatement(std::unique_ptr<StmtAST> stmt) {
        statements.push_back(std::move(stmt));
    }

    void accept(ASTVisitor& visitor) const override {
        visitor.visit(*this);
    }

    void print(int indent = 0) const override;
};

#endif // AST_H
