#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"
#include "llvm/IRGenerator.h"
#include "codegen/CodeGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

bool runInspectionPipeline(const std::string& sourceCode, const std::string& outputExeName) {
    std::cout << "===PHASE:LEXER===\n";
    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenizeAll();
    bool lexerError = false;

    std::cout << std::left 
              << std::setw(12) << "Location" 
              << std::setw(24) << "Token Type" 
              << "Lexeme\n";
    std::cout << "--------------------------------------------------------\n";
    for (const auto& tok : tokens) {
        std::string loc = "[" + std::to_string(tok.line) + ":" + std::to_string(tok.column) + "]";
        std::cout << std::left 
                  << std::setw(12) << loc 
                  << std::setw(24) << tokenTypeToString(tok.type) 
                  << "'" << tok.lexeme << "'\n";
        if (tok.type == TokenType::TOKEN_UNKNOWN) {
            std::cout << "  --> Lexical Error [line " << tok.line << ", col " << tok.column 
                      << "]: Unrecognized character '" << tok.lexeme << "'\n";
            lexerError = true;
        }
    }
    std::cout << "--------------------------------------------------------\n";
    if (lexerError) {
        std::cout << "Status: FAILED (Lexical errors detected)\n";
        return false;
    }
    std::cout << "Status: SUCCESS (" << tokens.size() << " tokens generated)\n";

    std::cout << "===PHASE:PARSER===\n";
    Parser parser(tokens);
    std::unique_ptr<ProgramAST> ast = parser.parse();

    if (parser.hasErrors()) {
        std::cout << "Status: FAILED (" << parser.getErrors().size() << " syntax error(s))\n";
        for (const auto& err : parser.getErrors()) {
            std::cout << "  " << err << "\n";
        }
        return false;
    }
    std::cout << "Status: SUCCESS (Grammar validated successfully)\n\n";
    std::cout << "Abstract Syntax Tree (AST Hierarchy):\n";
    std::cout << "--------------------------------------------------------\n";
    ast->print(1);
    std::cout << "--------------------------------------------------------\n";

    std::cout << "===PHASE:SEMANTIC===\n";
    SemanticAnalyzer semanticAnalyzer;
    if (!semanticAnalyzer.analyze(*ast)) {
        std::cout << "Status: FAILED (" << semanticAnalyzer.getErrors().size() << " semantic error(s))\n";
        for (const auto& err : semanticAnalyzer.getErrors()) {
            std::cout << "  " << err << "\n";
        }
        return false;
    }
    std::cout << "Status: SUCCESS (Scoping and type validation passed)\n\n";
    std::cout << "Symbol Table:\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << std::left 
              << std::setw(16) << "Name" 
              << std::setw(12) << "Type" 
              << std::setw(18) << "Declared At" 
              << "Initialized\n";
    std::cout << "--------------------------------------------------------\n";
    for (const auto& pair : semanticAnalyzer.getSymbolTable().getAllSymbols()) {
        const auto& sym = pair.second;
        std::string loc = "line " + std::to_string(sym.line) + ", col " + std::to_string(sym.column);
        std::cout << std::left 
                  << std::setw(16) << sym.name 
                  << std::setw(12) << sym.typeName 
                  << std::setw(18) << loc 
                  << (sym.isInitialized ? "yes" : "no") << "\n";
    }
    std::cout << "--------------------------------------------------------\n";

    std::cout << "===PHASE:LLVM===\n";
    IRGenerator irGen("MyLangModule");
    if (!irGen.generate(*ast)) {
        std::cout << "Status: FAILED (LLVM IR generation failed)\n";
        for (const auto& err : irGen.getErrors()) {
            std::cout << "  " << err << "\n";
        }
        return false;
    }
    std::cout << "Status: SUCCESS (LLVM Module verified)\n\n";
    std::cout << irGen.getIRString();

    std::cout << "===PHASE:CODEGEN===\n";
    CodeGenerator codeGen;
    std::string buildError;
    if (!codeGen.compileModule(irGen.getModule(), outputExeName, buildError)) {
        std::cout << "Status: FAILED (Native compilation failed)\n";
        std::cout << "  " << buildError << "\n";
        return false;
    }
    std::cout << "Status: SUCCESS (Native executable created)\n";
    std::cout << "Executable: " << outputExeName << "\n";
    std::cout << "Clang Driver: " << codeGen.getClangPath() << "\n\n";

    std::string progOut;
    codeGen.executeProgram(outputExeName, progOut);
    std::cout << "Execution Output:\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << progOut;
    std::cout << "--------------------------------------------------------\n";

    std::cout << "===OUTPUT===\n" << progOut;
    return true;
}

bool runPipeline(const std::string& testTitle, 
                 const std::string& sourceCode, 
                 const std::string& outputExeName, 
                 std::string& programOutput,
                 bool verbose = true) {
    if (verbose) {
        std::cout << "======================================================================\n";
        std::cout << "TEST CASE: " << testTitle << "\n";
        std::cout << "======================================================================\n";
        std::cout << "Source Code:\n";
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << sourceCode << "\n";
        std::cout << "----------------------------------------------------------------------\n";
    }

    Lexer lexer(sourceCode);
    std::vector<Token> tokens = lexer.tokenizeAll();

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::TOKEN_UNKNOWN) {
            std::cerr << "  --> Lexer Error [line " << tok.line << ", col " << tok.column 
                      << "]: Unrecognized character '" << tok.lexeme << "'\n";
            return false;
        }
    }
    if (verbose) std::cout << "  --> Lexer Status: OK (" << tokens.size() << " tokens)\n";

    Parser parser(tokens);
    std::unique_ptr<ProgramAST> ast = parser.parse();

    if (parser.hasErrors()) {
        std::cerr << "  --> Parser Status: FAILED (" << parser.getErrors().size() << " syntax error(s))\n";
        for (const auto& err : parser.getErrors()) {
            std::cerr << "      " << err << "\n";
        }
        return false;
    }
    if (verbose) std::cout << "  --> Parser Status: OK (AST generated successfully)\n";

    SemanticAnalyzer semanticAnalyzer;
    if (!semanticAnalyzer.analyze(*ast)) {
        std::cerr << "  --> Semantic Status: FAILED (" << semanticAnalyzer.getErrors().size() << " semantic error(s))\n";
        for (const auto& err : semanticAnalyzer.getErrors()) {
            std::cerr << "      " << err << "\n";
        }
        return false;
    }
    if (verbose) std::cout << "  --> Semantic Status: OK (All symbols valid)\n";

    IRGenerator irGen("MyLangModule");
    if (!irGen.generate(*ast)) {
        std::cerr << "  --> IR Generation Status: FAILED\n";
        for (const auto& err : irGen.getErrors()) {
            std::cerr << "      " << err << "\n";
        }
        return false;
    }
    if (verbose) {
        std::cout << "  --> IR Generation Status: OK\n\n";
        std::cout << "[Generated LLVM IR]:\n";
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << irGen.getIRString();
        std::cout << "----------------------------------------------------------------------\n";
    }

    CodeGenerator codeGen;
    std::string buildError;
    if (!codeGen.compileModule(irGen.getModule(), outputExeName, buildError)) {
        std::cerr << "  --> CodeGen Status: FAILED\n      " << buildError << "\n";
        return false;
    }
    if (verbose) std::cout << "  --> CodeGen Status: OK (Created " << outputExeName << ")\n\n";

    codeGen.executeProgram(outputExeName, programOutput);
    if (verbose) {
        std::cout << "[Program Execution Output]:\n";
        std::cout << programOutput;
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << "Result: COMPILATION & EXECUTION SUCCESSFUL!\n\n";
    }

    return true;
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && (std::string(argv[1]) == "--inspect" || std::string(argv[1]) == "--compile")) {
        std::string source = argv[2];
        std::string outExe = (argc >= 4) ? argv[3] : "temp_prog.exe";
        bool success = runInspectionPipeline(source, outExe);
        return success ? 0 : 1;
    }

    std::cout << "======================================================================\n";
    std::cout << "                    MyCompiler - Stages 1 to 5                        \n";
    std::cout << "    Lexer -> Parser -> AST -> Semantic -> LLVM IR -> Native CodeGen   \n";
    std::cout << "======================================================================\n\n";

    std::string output;

    std::string test1 =
        "int a = 10;\n"
        "int b = 20;\n"
        "int result = (a + b) * 2;\n"
        "print(result);";
    runPipeline("Test 1 — Basic Arithmetic", test1, "test1.exe", output);

    std::string test2 =
        "int count = 5;\n"
        "count = count + 10;\n"
        "int negativeVal = -15;\n"
        "print(count);\n"
        "print(negativeVal);";
    runPipeline("Test 2 — Assignment and Negative Numbers", test2, "test2.exe", output);

    std::string test3 =
        "int a = 10;\n"
        "int b = 20;\n"
        "int result = a < b;\n"
        "print(result);";
    runPipeline("Test 3 — Comparisons", test3, "test3.exe", output);

    std::string test4 =
        "int result = 10 / 3;\n"
        "print(result);";
    runPipeline("Test 4 — Integer Division", test4, "test4.exe", output);

    std::string test5 =
        "int result = 10 / 0;\n"
        "print(result);";
    std::cout << "======================================================================\n";
    std::cout << "TEST CASE: Test 5 — Division by Zero\n";
    std::cout << "======================================================================\n";
    std::cout << "Source Code:\n";
    std::cout << "----------------------------------------------------------------------\n";
    std::cout << test5 << "\n";
    std::cout << "----------------------------------------------------------------------\n";
    bool t5Success = runPipeline("Test 5 — Division by Zero", test5, "test5.exe", output, false);
    if (!t5Success) {
        std::cout << ">>> Pipeline halted as expected. No executable generated for division by zero.\n";
        std::cout << "Result: PASSED ERROR HANDLING CHECK!\n\n";
    }

    return 0;
}
