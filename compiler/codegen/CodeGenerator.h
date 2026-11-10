#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include <llvm/IR/Module.h>
#include <string>

class CodeGenerator {
public:
    explicit CodeGenerator(std::string clangPath = "C:\\LLVM-Dev\\bin\\clang.exe");

    bool compileModule(llvm::Module* module, 
                       const std::string& outputExePath, 
                       std::string& errorMessage);

    bool compileIRFile(const std::string& irFilePath, 
                       const std::string& outputExePath, 
                       std::string& errorMessage);

    int executeProgram(const std::string& exePath, std::string& output);

    const std::string& getClangPath() const { return clangPath; }

private:
    std::string clangPath;
};

#endif // CODE_GENERATOR_H
