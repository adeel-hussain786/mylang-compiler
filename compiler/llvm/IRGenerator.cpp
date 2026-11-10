#include "IRGenerator.h"

IRGenerator::IRGenerator(const std::string &moduleName)
    : context(), module(std::make_unique<llvm::Module>(moduleName, context)),
      builder(context), lastValue(nullptr) {}

llvm::Function *IRGenerator::getPrintfFunction() {
  llvm::Function *func = module->getFunction("printf");
  if (func) {
    return func;
  }

  llvm::FunctionType *printfType =
      llvm::FunctionType::get(builder.getInt32Ty(), {builder.getPtrTy()}, true);

  return llvm::Function::Create(printfType, llvm::Function::ExternalLinkage,
                                "printf", module.get());
}

llvm::Value *IRGenerator::getPrintFormatString() {
  return builder.CreateGlobalString("%d\n", "print_fmt");
}

bool IRGenerator::generate(const ProgramAST &program) {
  errors.clear();
  program.accept(*this);

  std::string verifyError;
  llvm::raw_string_ostream os(verifyError);
  if (llvm::verifyModule(*module, &os)) {
    errors.push_back("LLVM Verification Error: " + verifyError);
    return false;
  }

  return true;
}

void IRGenerator::printIR() const { module->print(llvm::outs(), nullptr); }

std::string IRGenerator::getIRString() const {
  std::string str;
  llvm::raw_string_ostream os(str);
  module->print(os, nullptr);
  return str;
}

bool IRGenerator::writeIRToFile(const std::string &filename) const {
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
  if (ec) {
    return false;
  }
  module->print(dest, nullptr);
  return true;
}

void IRGenerator::visit(const ProgramAST &node) {
  llvm::FunctionType *mainType =
      llvm::FunctionType::get(builder.getInt32Ty(), false);
  llvm::Function *mainFunc = llvm::Function::Create(
      mainType, llvm::Function::ExternalLinkage, "main", module.get());

  llvm::BasicBlock *entryBB =
      llvm::BasicBlock::Create(context, "entry", mainFunc);
  builder.SetInsertPoint(entryBB);

  namedValues.clear();

  for (const auto &stmt : node.statements) {
    if (stmt) {
      stmt->accept(*this);
    }
  }

  builder.CreateRet(builder.getInt32(0));
}

void IRGenerator::visit(const NumberExprAST &node) {
  lastValue = builder.getInt32(node.value);
}

void IRGenerator::visit(const VariableExprAST &node) {
  auto it = namedValues.find(node.name);
  if (it == namedValues.end()) {
    errors.push_back("IR Error: Variable '" + node.name +
                     "' has no stack allocation.");
    lastValue = nullptr;
    return;
  }
  lastValue = builder.CreateLoad(builder.getInt32Ty(), it->second, node.name);
}

void IRGenerator::visit(const UnaryExprAST &node) {
  if (node.operand) {
    node.operand->accept(*this);
  }
  if (!lastValue)
    return;

  if (node.op == "-") {
    lastValue = builder.CreateNeg(lastValue, "negtmp");
  }
}

void IRGenerator::visit(const BinaryExprAST &node) {
  if (!node.lhs || !node.rhs) {
    lastValue = nullptr;
    return;
  }

  node.lhs->accept(*this);
  llvm::Value *lhsVal = lastValue;

  node.rhs->accept(*this);
  llvm::Value *rhsVal = lastValue;

  if (!lhsVal || !rhsVal) {
    lastValue = nullptr;
    return;
  }

  if (node.op == "+") {
    lastValue = builder.CreateAdd(lhsVal, rhsVal, "addtmp");
  } else if (node.op == "-") {
    lastValue = builder.CreateSub(lhsVal, rhsVal, "subtmp");
  } else if (node.op == "*") {
    lastValue = builder.CreateMul(lhsVal, rhsVal, "multmp");
  } else if (node.op == "/") {
    lastValue = builder.CreateSDiv(lhsVal, rhsVal, "divtmp");
  } else if (node.op == "==") {
    llvm::Value *cmp = builder.CreateICmpEQ(lhsVal, rhsVal, "eqtmp");
    lastValue = builder.CreateZExt(cmp, builder.getInt32Ty(), "booltmp");
  } else if (node.op == "!=") {
    llvm::Value *cmp = builder.CreateICmpNE(lhsVal, rhsVal, "netmp");
    lastValue = builder.CreateZExt(cmp, builder.getInt32Ty(), "booltmp");
  } else if (node.op == "<") {
    llvm::Value *cmp = builder.CreateICmpSLT(lhsVal, rhsVal, "slttmp");
    lastValue = builder.CreateZExt(cmp, builder.getInt32Ty(), "booltmp");
  } else if (node.op == "<=") {
    llvm::Value *cmp = builder.CreateICmpSLE(lhsVal, rhsVal, "sletmp");
    lastValue = builder.CreateZExt(cmp, builder.getInt32Ty(), "booltmp");
  } else if (node.op == ">") {
    llvm::Value *cmp = builder.CreateICmpSGT(lhsVal, rhsVal, "sgttmp");
    lastValue = builder.CreateZExt(cmp, builder.getInt32Ty(), "booltmp");
  } else if (node.op == ">=") {
    llvm::Value *cmp = builder.CreateICmpSGE(lhsVal, rhsVal, "sgetmp");
    lastValue = builder.CreateZExt(cmp, builder.getInt32Ty(), "booltmp");
  } else {
    errors.push_back("IR Error: Unsupported binary operator '" + node.op +
                     "'.");
    lastValue = nullptr;
  }
}

void IRGenerator::visit(const VarDeclStmtAST &node) {
  llvm::AllocaInst *allocaInst =
      builder.CreateAlloca(builder.getInt32Ty(), nullptr, node.varName);
  namedValues[node.varName] = allocaInst;

  if (node.initializer) {
    node.initializer->accept(*this);
    if (lastValue) {
      builder.CreateStore(lastValue, allocaInst);
    }
  } else {
    builder.CreateStore(builder.getInt32(0), allocaInst);
  }
}

void IRGenerator::visit(const AssignStmtAST &node) {
  auto it = namedValues.find(node.varName);
  if (it == namedValues.end()) {
    errors.push_back("IR Error: Assignment to unallocated variable '" +
                     node.varName + "'.");
    return;
  }

  if (node.value) {
    node.value->accept(*this);
    if (lastValue) {
      builder.CreateStore(lastValue, it->second);
    }
  }
}

void IRGenerator::visit(const PrintStmtAST &node) {
  if (node.expr) {
    node.expr->accept(*this);
    if (lastValue) {
      llvm::Function *printfFunc = getPrintfFunction();
      llvm::Value *fmt = getPrintFormatString();
      builder.CreateCall(printfFunc, {fmt, lastValue});
    }
  }
}
