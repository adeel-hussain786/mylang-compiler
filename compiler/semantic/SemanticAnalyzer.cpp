#include "SemanticAnalyzer.h"
#include <iostream>

bool SymbolTable::declare(const std::string& name, const std::string& typeName, int line, int column, bool isInitialized) {
    if (isDeclared(name)) {
        return false;
    }
    symbols[name] = Symbol{name, typeName, line, column, isInitialized};
    return true;
}

bool SymbolTable::isDeclared(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return &it->second;
    }
    return nullptr;
}

void SymbolTable::markInitialized(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        it->second.isInitialized = true;
    }
}

void SemanticAnalyzer::recordError(int line, int column, const std::string& message) {
    errors.push_back("Semantic Error [line " + std::to_string(line) + 
                     ", col " + std::to_string(column) + "]: " + message);
}

bool SemanticAnalyzer::analyze(const ProgramAST& program) {
    errors.clear();
    program.accept(*this);
    return !hasErrors();
}

void SemanticAnalyzer::visit(const ProgramAST& node) {
    for (const auto& stmt : node.statements) {
        if (stmt) {
            stmt->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(const VarDeclStmtAST& node) {
    if (node.typeName != "int") {
        recordError(node.line, node.column, "Unsupported type '" + node.typeName + "'. MyLang only supports 'int'.");
    }

    if (symbolTable.isDeclared(node.varName)) {
        const Symbol* existing = symbolTable.lookup(node.varName);
        std::string prevLoc = existing ? " (first declared at line " + std::to_string(existing->line) + ")" : "";
        recordError(node.line, node.column, "Redeclaration of variable '" + node.varName + "'" + prevLoc + ".");
        return;
    }

    bool hasInit = (node.initializer != nullptr);
    if (hasInit) {
        node.initializer->accept(*this);
    }

    symbolTable.declare(node.varName, node.typeName, node.line, node.column, hasInit);
}

void SemanticAnalyzer::visit(const AssignStmtAST& node) {
    if (!symbolTable.isDeclared(node.varName)) {
        recordError(node.line, node.column, "Assignment to undeclared variable '" + node.varName + 
                    "'. Variables must be declared with 'int' before use.");
    } else {
        symbolTable.markInitialized(node.varName);
    }

    if (node.value) {
        node.value->accept(*this);
    }
}

void SemanticAnalyzer::visit(const PrintStmtAST& node) {
    if (node.expr) {
        node.expr->accept(*this);
    }
}

void SemanticAnalyzer::visit(const VariableExprAST& node) {
    if (!symbolTable.isDeclared(node.name)) {
        recordError(node.line, node.column, "Use of undeclared variable '" + node.name + 
                    "'. Variables must be declared before being referenced in expressions.");
    }
}

void SemanticAnalyzer::visit(const BinaryExprAST& node) {
    if (node.lhs) {
        node.lhs->accept(*this);
    }
    if (node.rhs) {
        node.rhs->accept(*this);
    }

    if (node.op == "/") {
        if (auto num = dynamic_cast<const NumberExprAST*>(node.rhs.get())) {
            if (num->value == 0) {
                recordError(node.line, node.column, "Division by zero detected in constant expression.");
            }
        }
    }
}

void SemanticAnalyzer::visit(const UnaryExprAST& node) {
    if (node.operand) {
        node.operand->accept(*this);
    }
}

void SemanticAnalyzer::visit(const NumberExprAST& node) {
    (void)node;
}
