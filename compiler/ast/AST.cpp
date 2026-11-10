#include "AST.h"
#include <iostream>

static void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

void NumberExprAST::print(int indent) const {
    printIndent(indent);
    std::cout << "NumberExpr: " << value << " [line " << line << ", col " << column << "]\n";
}

void VariableExprAST::print(int indent) const {
    printIndent(indent);
    std::cout << "VariableExpr: '" << name << "' [line " << line << ", col " << column << "]\n";
}

void BinaryExprAST::print(int indent) const {
    printIndent(indent);
    std::cout << "BinaryExpr: '" << op << "' [line " << line << ", col " << column << "]\n";
    if (lhs) lhs->print(indent + 1);
    if (rhs) rhs->print(indent + 1);
}

void UnaryExprAST::print(int indent) const {
    printIndent(indent);
    std::cout << "UnaryExpr: '" << op << "' [line " << line << ", col " << column << "]\n";
    if (operand) operand->print(indent + 1);
}

void VarDeclStmtAST::print(int indent) const {
    printIndent(indent);
    std::cout << "VarDecl: " << typeName << " " << varName << " [line " << line << ", col " << column << "]\n";
    if (initializer) {
        initializer->print(indent + 1);
    } else {
        printIndent(indent + 1);
        std::cout << "<uninitialized>\n";
    }
}

void AssignStmtAST::print(int indent) const {
    printIndent(indent);
    std::cout << "AssignStmt: " << varName << " = [line " << line << ", col " << column << "]\n";
    if (value) value->print(indent + 1);
}

void PrintStmtAST::print(int indent) const {
    printIndent(indent);
    std::cout << "PrintStmt [line " << line << ", col " << column << "]\n";
    if (expr) expr->print(indent + 1);
}

void ProgramAST::print(int indent) const {
    printIndent(indent);
    std::cout << "ProgramAST (" << statements.size() << " statements)\n";
    for (const auto& stmt : statements) {
        stmt->print(indent + 1);
    }
}
