#include "CodeGenerator.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <cstdio>
#include <iostream>

CodeGenerator::CodeGenerator(std::string clangPath)
    : clangPath(std::move(clangPath)) {}

bool CodeGenerator::compileModule(llvm::Module* module, 
                                  const std::string& outputExePath, 
                                  std::string& errorMessage) {
    if (!module) {
        errorMessage = "Error: Null LLVM Module provided for code generation.";
        return false;
    }

    std::string tempIR = outputExePath + ".ll";
    std::error_code ec;
    llvm::raw_fd_ostream dest(tempIR, ec, llvm::sys::fs::OF_None);
    if (ec) {
        errorMessage = "Error: Failed to write intermediate IR file: " + ec.message();
        return false;
    }

    module->print(dest, nullptr);
    dest.flush();

    return compileIRFile(tempIR, outputExePath, errorMessage);
}

bool CodeGenerator::compileIRFile(const std::string& irFilePath, 
                                 const std::string& outputExePath, 
                                 std::string& errorMessage) {
    std::string cmd = "\"" + clangPath + "\" \"" + irFilePath + "\" -o \"" + outputExePath + "\" -Wno-override-module";
    std::string wrappedCmd = "\"" + cmd + "\" 2>&1";

    FILE* pipe = _popen(wrappedCmd.c_str(), "r");
    if (!pipe) {
        errorMessage = "Error: Failed to execute clang compiler at " + clangPath;
        return false;
    }

    char buffer[256];
    std::string clangOutput;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        clangOutput += buffer;
    }

    int exitCode = _pclose(pipe);
    if (exitCode != 0) {
        errorMessage = "Clang Linker Error (code " + std::to_string(exitCode) + "):\n" + clangOutput;
        return false;
    }

    return true;
}

int CodeGenerator::executeProgram(const std::string& exePath, std::string& output) {
    output.clear();
    std::string cmd = (exePath.find('/') == std::string::npos && exePath.find('\\') == std::string::npos) 
                      ? ".\\" + exePath 
                      : exePath;
    std::string wrappedCmd = "\"" + cmd + "\" 2>&1";

    FILE* pipe = _popen(wrappedCmd.c_str(), "r");
    if (!pipe) {
        output = "Error: Failed to execute binary at " + exePath;
        return -1;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output += buffer;
    }

    return _pclose(pipe);
}
