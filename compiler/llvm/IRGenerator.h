#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "ast/AST.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

class IRGenerator : public ASTVisitor {
public:
    explicit IRGenerator(const std::string& moduleName = "MyLangModule");

    bool generate(const ProgramAST& program);
    void printIR() const;
    std::string getIRString() const;
    bool writeIRToFile(const std::string& filename) const;

    llvm::Module* getModule() const { return module.get(); }
    llvm::LLVMContext& getContext() { return context; }
    const std::vector<std::string>& getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty(); }

    void visit(const NumberExprAST& node) override;
    void visit(const VariableExprAST& node) override;
    void visit(const BinaryExprAST& node) override;
    void visit(const UnaryExprAST& node) override;
    void visit(const VarDeclStmtAST& node) override;
    void visit(const AssignStmtAST& node) override;
    void visit(const PrintStmtAST& node) override;
    void visit(const ProgramAST& node) override;

private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    std::unordered_map<std::string, llvm::AllocaInst*> namedValues;
    llvm::Value* lastValue;
    std::vector<std::string> errors;

    llvm::Function* getPrintfFunction();
    llvm::Value* getPrintFormatString();
};

#endif // IR_GENERATOR_H
