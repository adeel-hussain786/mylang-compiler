# MyCompiler — A Custom C++ & LLVM Compiler for MyLang

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![LLVM](https://img.shields.io/badge/LLVM-23.1.5-yellow.svg)](https://llvm.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-brightgreen.svg)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20x64-lightgrey.svg)](https://www.microsoft.com/windows)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

**MyCompiler** is an end-to-end compiler for **MyLang**, a statically typed imperative programming language designed for Compiler Construction studies. 

The compiler frontend—comprising the **Lexer**, **Recursive Descent Parser**, **Abstract Syntax Tree (AST)**, and **Semantic Analyzer**—is implemented completely from scratch in modern **C++17**. The backend leverages the **LLVM C++ API** for Intermediate Representation (IR) generation, optimization, and target-native executable machine code generation.

The project includes both a command-line interface and a modern, responsive **Web-based IDE** featuring an **Individual Compiler Phase Viewer** to inspect the live internal state across all compiler stages.

---

## Table of Contents

1. [Features](#features)
2. [Compiler Architecture](#compiler-architecture)
3. [MyLang Language Specification](#mylang-language-specification)
4. [Project Structure](#project-structure)
5. [Prerequisites & Environment Setup](#prerequisites--environment-setup)
6. [Build Instructions](#build-instructions)
7. [Running the Compiler](#running-the-compiler)
   - [Automated Verification Suite](#1-automated-verification-suite)
   - [CLI Pipeline & Phase Inspection](#2-cli-pipeline--phase-inspection)
   - [Interactive Web IDE](#3-interactive-web-ide)
8. [Individual Phase Inspector Details](#individual-phase-inspector-details)
9. [Test Suite Walkthrough](#test-suite-walkthrough)
10. [Git & Repository Preparation](#git--repository-preparation)

---

## Features

- **Hand-Crafted Frontend**:
  - Deterministic Lexer with accurate line and column source location tracking.
  - Recursive Descent Parser implementing strict operator precedence and associativity.
  - Strongly typed, hierarchical Abstract Syntax Tree (AST) with tree visualizer.
  - Semantic Analyzer with scoped Symbol Table, type enforcement, duplicate declaration guards, and constant-folding division-by-zero checks.
- **LLVM Backend**:
  - LLVM `IRBuilder` generating LLVM Intermediate Representation.
  - 32-bit signed integer (`i32`) arithmetic, comparisons (`i1` to `i32` zero-extension), and runtime I/O bindings (`printf`).
  - Native machine code compilation via Clang/LLVM target toolchain producing standalone Windows PE executables (`.exe`).
- **Interactive Web IDE & Visualizer**:
  - Dark-mode responsive web interface with preset sample programs.
  - Clickable compiler pipeline steps allowing interactive inspection of the outputs of each phase: Lexer tokens, AST hierarchy, Symbol Table, LLVM IR, and native code generation logs.

---

## Compiler Architecture

```
+--------------------------------------------------------------------------+
|                                MyLang Source                             |
+--------------------------------------------------------------------------+
                                     |
                                     v
+--------------------------------------------------------------------------+
| Stage 1: Lexical Analysis (Lexer)                                        |
| Transforms source characters into a stream of typed Tokens with location |
+--------------------------------------------------------------------------+
                                     |
                                     v
+--------------------------------------------------------------------------+
| Stage 2: Syntax Analysis (Parser)                                        |
| Validates grammar using Recursive Descent and constructs the AST         |
+--------------------------------------------------------------------------+
                                     |
                                     v
+--------------------------------------------------------------------------+
| Stage 3: Semantic Analysis (SemanticAnalyzer)                            |
| Manages Symbol Table, validates declarations, and detects semantic errors|
+--------------------------------------------------------------------------+
                                     |
                                     v
+--------------------------------------------------------------------------+
| Stage 4: LLVM IR Generation (IRGenerator)                                |
| Translates AST nodes into LLVM IR using llvm::IRBuilder and llvm::Module |
+--------------------------------------------------------------------------+
                                     |
                                     v
+--------------------------------------------------------------------------+
| Stage 5: Native Code Generation (CodeGenerator)                          |
| Compiles LLVM IR into standalone native Windows x64 binary (.exe)       |
+--------------------------------------------------------------------------+
                                     |
                                     v
+--------------------------------------------------------------------------+
|                             Program Output                               |
+--------------------------------------------------------------------------+
```

---

## MyLang Language Specification

### Grammar (EBNF)

```ebnf
program         ::= statement* EOF ;

statement       ::= var_decl
                  | assignment
                  | print_stmt ;

var_decl        ::= "int" IDENTIFIER ( "=" expression )? ";" ;
assignment      ::= IDENTIFIER "=" expression ";" ;
print_stmt      ::= "print" "(" expression ")" ";" ;

expression      ::= equality ;
equality        ::= comparison ( ( "==" | "!=" ) comparison )* ;
comparison      ::= term ( ( "<" | "<=" | ">" | ">=" ) term )* ;
term            ::= factor ( ( "+" | "-" ) factor )* ;
factor          ::= unary ( ( "*" | "/" ) unary )* ;
unary           ::= "-" unary | primary ;
primary         ::= INTEGER_LITERAL
                  | IDENTIFIER
                  | "(" expression ")" ;
```

### Language Characteristics

| Feature | Syntax / Semantics | Example |
| :--- | :--- | :--- |
| **Types** | 32-bit signed integer (`int`) | `int a = 10;` |
| **Variable Declarations** | Explicit type with optional initial value (defaults to 0) | `int x;`<br>`int y = 42;` |
| **Assignments** | Reassignment of declared variables | `x = 25;` |
| **Arithmetic Operators** | `+`, `-`, `*`, `/` (Integer division truncating towards zero) | `int res = (10 + 20) * 2;` |
| **Unary Minus** | Inversion of sign | `int neg = -15;` |
| **Relational Operators** | `==`, `!=`, `<`, `<=`, `>`, `>=` (Evaluate to `1` for true, `0` for false) | `int cmp = a < b;` |
| **Standard Output** | Built-in print statement printing integer values followed by newline | `print(a);` |
| **Comments** | Single-line comments starting with `//` | `// This is a comment` |

### Operator Precedence

| Level | Operators | Description | Associativity |
| :---: | :---: | :--- | :---: |
| **1 (Highest)** | `()` | Parentheses | Non-associative |
| **2** | `-` | Unary negation | Right-to-left |
| **3** | `*`, `/` | Multiplication, Integer Division | Left-to-right |
| **4** | `+`, `-` | Addition, Subtraction | Left-to-right |
| **5** | `<`, `<=`, `>`, `>=` | Relational Comparisons | Left-to-right |
| **6** | `==`, `!=` | Equality Comparisons | Left-to-right |
| **7 (Lowest)**| `=` | Assignment | Right-to-left |

---

## Project Structure

```
MyCompiler/
├── .gitignore                   # Ignores build artifacts, object files, binaries, and caches
├── CMakeLists.txt               # CMake configuration linking C++17 with LLVM Core/Support
├── README.md                    # Project documentation and specifications
├── server.py                    # Python HTTP server and bridge between Web IDE and compiler
├── compiler/                    # C++ Compiler source code
│   ├── main.cpp                 # Entry point: CLI driver, test suite, and inspection pipeline
│   ├── lexer/
│   │   ├── Token.h              # Token types, Token struct, and string conversion
│   │   ├── Lexer.h              # Lexer class declaration
│   │   └── Lexer.cpp            # Tokenization logic with character scanner
│   ├── parser/
│   │   ├── Parser.h             # Recursive Descent Parser declaration
│   │   └── Parser.cpp           # Grammar parsing and syntax error recovery
│   ├── ast/
│   │   ├── AST.h                # AST Node hierarchy (ExprAST, StmtAST, ProgramAST)
│   │   └── AST.cpp              # Formatted hierarchical AST printing logic
│   ├── semantic/
│   │   ├── SemanticAnalyzer.h   # Symbol Table and semantic analysis declaration
│   │   └── SemanticAnalyzer.cpp # Scope checking, type safety, and division-by-zero checks
│   ├── llvm/
│   │   ├── IRGenerator.h        # LLVM IR Generator class declaration
│   │   └── IRGenerator.cpp      # AST-to-LLVM IR lowering with llvm::IRBuilder
│   └── codegen/
│       ├── CodeGenerator.h      # Native Code Generator declaration
│       └── CodeGenerator.cpp    # Clang integration emitting native Windows executables
└── frontend/                    # Web IDE frontend assets
    ├── index.html               # Web IDE layout, code editor, and phase inspector tabs
    ├── style.css                # Dark theme UI styling and responsive layout
    └── script.js                # Editor interactions, sample loader, and API client
```

---

## Prerequisites & Environment Setup

### Required Tools
- **Operating System**: Windows 10 or Windows 11 (x64)
- **Compiler**: Visual Studio 2022 / 2026 with MSVC C++ toolchain (`cl.exe`) supporting C++17
- **Build System**: CMake 3.20 or newer
- **Backend / LLVM**: LLVM 23.1.5 (Installed at `C:\LLVM-Dev` or set via `LLVM_DIR`)
- **Runtime Compiler**: `clang.exe` bundled with the LLVM installation
- **Python**: Python 3.8+ (for running `server.py` to launch the Web IDE)

### Configuring the Environment

Ensure `cmake`, `git`, `python`, and `clang` are accessible in your system `PATH`:

```powershell
cmake --version
python --version
clang --version
```

If LLVM is in a custom path, configure `LLVM_DIR` to point to `lib/cmake/llvm`:
```powershell
$env:LLVM_DIR = "C:\LLVM-Dev\lib\cmake\llvm"
```

---

## Build Instructions

1. **Clone the repository**:
   ```powershell
   git clone https://github.com/AdeelHussain/MyCompiler.git
   cd MyCompiler
   ```

2. **Generate build files with CMake**:
   ```powershell
   cmake -B build -S .
   ```

3. **Compile the project**:
   ```powershell
   cmake --build build --config Debug
   ```

Upon completion, the compiled executable will be located at:
```
build/Debug/MyCompiler.exe
```

---

## Running the Compiler

### 1. Automated Verification Suite
Run the executable directly without arguments to execute the built-in test suite:

```powershell
.\build\Debug\MyCompiler.exe
```

This runs 5 comprehensive integration tests exercising all pipeline stages and prints results to the terminal.

---

### 2. CLI Pipeline & Phase Inspection
You can compile and inspect arbitrary MyLang code from the command line using the `--inspect` or `--compile` flags:

```powershell
.\build\Debug\MyCompiler.exe --inspect "int a = 10; int b = 20; print((a + b) * 2);" program.exe
```

Output includes:
- Tagged phase markers (`===PHASE:LEXER===`, `===PHASE:PARSER===`, etc.)
- Token stream with line and column tracking
- Hierarchical AST indentation
- Symbol Table dump
- Validated LLVM IR
- Standalone executable generation status and runtime execution output

---

### 3. Interactive Web IDE
Launch the local web server to access the modern IDE in your browser:

```powershell
python server.py
```

Open your browser and navigate to:
```
http://localhost:8080
```

#### Web IDE Capabilities:
- **Source Code Editor**: Syntax-highlighted code editing with line numbers and quick sample presets.
- **Run Pipeline**: Click **Run** to compile and execute the program, viewing the terminal output in real time.
- **Phase Inspector Navigation**: Click on any of the five pipeline buttons:
  - **1. Lexer**: View token stream, token types, and source coordinates.
  - **2. Parser**: Inspect the Abstract Syntax Tree hierarchy.
  - **3. Semantic Analysis**: View the Symbol Table and semantic validation status.
  - **4. LLVM IR**: Inspect generated LLVM IR instructions.
  - **5. Code Generation**: View native executable generation details and program runtime output.

---

## Individual Phase Inspector Details

| Phase | Internal Component | Output Displayed |
| :--- | :--- | :--- |
| **Lexer** | `Lexer.cpp` / `Token.h` | Token table displaying token type (`KEYWORD_INT`, `IDENTIFIER`, `ASSIGN`, etc.), raw lexemes, line numbers, and column offsets. |
| **Parser** | `Parser.cpp` / `AST.h` | Abstract Syntax Tree visualizer with indented node hierarchy representing variable declarations, binary expressions, and print statements. |
| **Semantic Analysis** | `SemanticAnalyzer.cpp` | Symbol Table details listing variable names, types (`i32`), declaration line/column, and initialization state, along with validation checks. |
| **LLVM IR** | `IRGenerator.cpp` | LLVM Intermediate Representation text with allocated stack space (`alloca`), loads, arithmetic operations, comparisons, and `printf` calls. |
| **Code Generation** | `CodeGenerator.cpp` | Native target machine information, Clang compilation log, generated executable metadata, and the program's runtime standard output. |

---

## Test Suite Walkthrough

The compiler includes built-in verification covering core scenarios:

### Test 1 — Basic Arithmetic & Precedence
```c
int a = 10;
int b = 20;
int result = (a + b) * 2;
print(result);
```
- **Expected Output**: `60`
- **Verification**: Tests variable initialization, parenthesized expressions, addition, multiplication precedence, and `print()`.

---

### Test 2 — Variable Assignment & Unary Minus
```c
int count = 5;
count = count + 10;
int negativeVal = -15;
print(count);
print(negativeVal);
```
- **Expected Output**: `15` and `-15`
- **Verification**: Tests re-assignment to existing variables, arithmetic accumulation, unary minus evaluation, and multiple outputs.

---

### Test 3 — Relational Comparisons
```c
int a = 10;
int b = 20;
int result = a < b;
print(result);
```
- **Expected Output**: `1`
- **Verification**: Verifies relational comparison `<` returning boolean `1`, stored as `i32` and printed.

---

### Test 4 — Integer Division Truncation
```c
int result = 10 / 3;
print(result);
```
- **Expected Output**: `3`
- **Verification**: Confirms integer division drops remainders, matching standard C/C++ integer semantics.

---

### Test 5 — Constant Fold Division-by-Zero Protection
```c
int result = 10 / 0;
print(result);
```
- **Expected Output**: Semantic analysis failure:
  ```
  Semantic Error: Division by zero detected in expression.
  ```
- **Verification**: Demonstrates static semantic analysis intercepting division by zero before intermediate code generation, preventing undefined runtime behavior.

---

## Git & Repository Preparation

The repository includes a tailored `.gitignore` to maintain a clean git history:

- Excludes Visual Studio artifacts (`.vs/`, `*.sln`, `*.vcxproj`, `*.pdb`, `*.ilk`).
- Excludes CMake build directories (`build/`, `CMakeFiles/`, `CMakeCache.txt`).
- Excludes binary executables (`*.exe`, `*.obj`, `*.o`, `*.lib`).
- Excludes temporary LLVM files (`*.ll`, `*.bc`, `web_run*`, `test*.exe*`).
- Excludes Python caches (`__pycache__/`).

To initialize git and push to your remote GitHub repository:

```powershell
git init
git add .
git commit -m "Initial commit: Complete MyCompiler C++ & LLVM pipeline with Web IDE"
git branch -M main
git remote add origin https://github.com/<your-username>/MyCompiler.git
git push -u origin main
```

