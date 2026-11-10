# MyCompiler --- A Custom C++17 & LLVM Compiler for MyLang

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![LLVM](https://img.shields.io/badge/LLVM-23.1.5-yellow.svg)
![CMake](https://img.shields.io/badge/CMake-3.20%2B-brightgreen.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%20x64-lightgrey.svg)
![License](https://img.shields.io/badge/License-MIT-green.svg)

**MyCompiler** is an educational compiler for **MyLang**, a small,
statically typed, imperative programming language created to demonstrate
the main concepts of Compiler Construction.

The compiler frontend is implemented from scratch in **C++17** and
includes:

-   Lexical Analysis (Lexer)
-   Syntax Analysis (Recursive Descent Parser)
-   Abstract Syntax Tree (AST)
-   Semantic Analysis and Symbol Table

The backend uses the **LLVM C++ API** to generate LLVM Intermediate
Representation (LLVM IR) and compile programs into native Windows
executables.

The project provides two ways to use the compiler:

1.  **Command-Line Interface (CLI):** compile and inspect MyLang
    programs from the terminal.
2.  **Web-Based IDE:** write code, run the compiler pipeline, and
    inspect the output of each compiler phase through a browser
    interface.

------------------------------------------------------------------------

## Table of Contents

-   [Features](#features)
-   [Compiler Architecture](#compiler-architecture)
-   [MyLang Language](#mylang-language)
-   [Project Structure](#project-structure)
-   [Prerequisites](#prerequisites)
-   [Installation and Environment
    Setup](#installation-and-environment-setup)
-   [Build the Project](#build-the-project)
-   [Run the Compiler](#run-the-compiler)
    -   [Run the Automated Tests](#1-run-the-automated-tests)
    -   [Use the CLI](#2-use-the-cli)
    -   [Launch the Web IDE](#3-launch-the-web-ide)
-   [Understanding the Compiler
    Phases](#understanding-the-compiler-phases)
-   [Example Programs](#example-programs)
-   [Troubleshooting](#troubleshooting)
-   [License](#license)

------------------------------------------------------------------------

## Features

### Compiler Frontend

-   **Lexer:** converts source-code characters into tokens and tracks
    line and column positions.
-   **Parser:** uses Recursive Descent Parsing to validate syntax and
    operator precedence.
-   **AST:** creates a hierarchical representation of the source
    program.
-   **Semantic Analyzer:** checks declarations, variable usage, types,
    and division-by-zero cases.
-   **Symbol Table:** stores information about declared variables.

### LLVM Backend

-   Generates LLVM IR using LLVM's `IRBuilder` API.
-   Supports 32-bit signed integer (`i32`) operations.
-   Supports arithmetic and comparison operations.
-   Uses `printf` for standard output.
-   Uses the Clang/LLVM toolchain to generate native Windows
    executables.

### Web-Based IDE

-   Browser-based code editor with sample programs.
-   Run button to execute the compiler pipeline.
-   Individual phase inspection for Lexer, Parser, Semantic Analysis,
    LLVM IR, and Code Generation.
-   Displays compiler results and program output.

------------------------------------------------------------------------

## Compiler Architecture

The compiler processes a MyLang program through five main stages:

``` text
             MyLang Source Code
                     |
                     v
        +--------------------------+
        | 1. Lexical Analysis      |
        |         Lexer            |
        +--------------------------+
                     |
                     v
        +--------------------------+
        | 2. Syntax Analysis       |
        |   Recursive Descent      |
        |         Parser           |
        +--------------------------+
                     |
                     v
        +--------------------------+
        | 3. Semantic Analysis     |
        | Symbol Table & Type Check|
        +--------------------------+
                     |
                     v
        +--------------------------+
        | 4. LLVM IR Generation    |
        |       IRGenerator        |
        +--------------------------+
                     |
                     v
        +--------------------------+
        | 5. Native Code Generation|
        |       CodeGenerator      |
        +--------------------------+
                     |
                     v
              Program Output
```

  -----------------------------------------------------------------------
  Phase                   Responsibility          Main output
  ----------------------- ----------------------- -----------------------
  1\. Lexer               Converts characters     Token stream
                          into tokens             

  2\. Parser              Checks grammar and      AST
                          builds the syntax tree  

  3\. Semantic Analysis   Checks declarations and Symbol Table and
                          semantic rules          validation result

  4\. LLVM IR Generation  Converts AST operations LLVM IR
                          into LLVM instructions  

  5\. Code Generation     Compiles LLVM IR for    Native executable
                          the target machine      
  -----------------------------------------------------------------------

------------------------------------------------------------------------

## MyLang Language

MyLang supports basic integer-based programming constructs.

### Supported Features

  -------------------------------------------------------------------------
  Feature                 Description             Example
  ----------------------- ----------------------- -------------------------
  Data type               32-bit signed integer   `int age = 20;`

  Variable declaration    Declare a variable,     `int number;`
                          with an optional        
                          initial value           

  Assignment              Update an existing      `number = 25;`
                          variable                

  Addition                Add two values          `int x = 10 + 5;`

  Subtraction             Subtract values         `int x = 10 - 5;`

  Multiplication          Multiply values         `int x = 10 * 5;`

  Division                Integer division        `int x = 10 / 2;`

  Unary minus             Change a value's sign   `int x = -10;`

  Comparisons             Compare two values      `int result = 10 < 20;`

  Output                  Print an integer        `print(number);`

  Comments                Single-line comments    `// This is a comment`
  -------------------------------------------------------------------------

Comparison expressions return `1` for true and `0` for false. Integer
division truncates the fractional part.

### Grammar (EBNF)

``` ebnf
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

### Operator Precedence

From highest to lowest:

    Level Operators              Associativity
  ------- ---------------------- ---------------
        1 `()`                   Parentheses
        2 Unary `-`              Right-to-left
        3 `*`, `/`               Left-to-right
        4 `+`, `-`               Left-to-right
        5 `<`, `<=`, `>`, `>=`   Left-to-right
        6 `==`, `!=`             Left-to-right

Assignment is supported as a statement using `identifier = expression;`.

------------------------------------------------------------------------

## Project Structure

``` text
MyCompiler/
├── CMakeLists.txt
├── README.md
├── server.py
├── compiler/
│   ├── main.cpp
│   ├── lexer/
│   │   ├── Token.h
│   │   ├── Lexer.h
│   │   └── Lexer.cpp
│   ├── parser/
│   │   ├── Parser.h
│   │   └── Parser.cpp
│   ├── ast/
│   │   ├── AST.h
│   │   └── AST.cpp
│   ├── semantic/
│   │   ├── SemanticAnalyzer.h
│   │   └── SemanticAnalyzer.cpp
│   ├── llvm/
│   │   ├── IRGenerator.h
│   │   └── IRGenerator.cpp
│   └── codegen/
│       ├── CodeGenerator.h
│       └── CodeGenerator.cpp
└── frontend/
    ├── index.html
    ├── style.css
    └── script.js
```

------------------------------------------------------------------------

## Prerequisites

This project is documented for **Windows 10/11 (64-bit)**.

Install the following tools before building the project:

  -----------------------------------------------------------------------
  Tool                    Requirement             Purpose
  ----------------------- ----------------------- -----------------------
  Visual Studio           Visual Studio 2022 or a C++ compiler
                          compatible version with 
                          the MSVC C++ toolchain  

  MSVC                    C++ build tools and     Compiles the C++ source
                          Windows SDK             

  CMake                   Version 3.20 or newer   Generates build files

  LLVM                    Version 23.1.5, as used LLVM libraries and
                          by this project         development files

  Clang                   Available through the   Native code generation
                          LLVM installation       

  Python                  Version 3.8 or newer    Runs the Web IDE server

  Git                     Installed and available Clones the repository
                          in PATH                 
  -----------------------------------------------------------------------

### Visual Studio components

When installing Visual Studio, make sure the **Desktop development with
C++** workload is selected. It should provide the MSVC compiler and
Windows SDK.

> **Important:** The project uses the LLVM C++ API. Installing only
> Clang is not enough; the LLVM development libraries and CMake
> configuration files are also required.

------------------------------------------------------------------------

## Installation and Environment Setup

Follow these steps on a Windows machine.

### Step 1 --- Install the required tools

Install Visual Studio with the C++ workload, CMake, Git, Python, and the
LLVM distribution required by the project.

After installation, open **Developer PowerShell for Visual Studio**.
This terminal configures the MSVC environment needed by CMake.

### Step 2 --- Check the installations

Run these commands:

``` powershell
cmake --version
git --version
python --version
clang --version
```

Check that the LLVM installation contains its CMake package
configuration file. For the example installation path used in this
README, it should be:

``` text
C:\LLVM-Dev\lib\cmake\llvm\LLVMConfig.cmake
```

If LLVM is installed in another directory, use that directory in the
next step.

### Step 3 --- Configure LLVM

If LLVM is installed at `C:\LLVM-Dev`, set `LLVM_DIR` in the current
PowerShell session:

``` powershell
$env:LLVM_DIR = "C:\LLVM-Dev\lib\cmake\llvm"
```

You can verify the variable with:

``` powershell
echo $env:LLVM_DIR
```

If you installed LLVM somewhere else, replace `C:\LLVM-Dev` with your
actual LLVM installation directory.

### Step 4 --- Clone the repository

``` powershell
git clone https://github.com/AdeelHussain/MyCompiler.git
cd MyCompiler
```

### Step 5 --- Configure the project with CMake

Run this command from the project root:

``` powershell
cmake -B build -S . -DLLVM_DIR="$env:LLVM_DIR"
```

This asks CMake to configure the project and locate the LLVM package.

If CMake cannot find LLVM, confirm that `LLVM_DIR` points to the folder
containing `LLVMConfig.cmake`.

------------------------------------------------------------------------

## Build the Project

After CMake configuration completes successfully, compile the project:

``` powershell
cmake --build build --config Debug
```

When the build succeeds, the executable is expected at:

``` text
build/Debug/MyCompiler.exe
```

If you use a different CMake generator or build configuration, the
output directory may differ.

------------------------------------------------------------------------

## Run the Compiler

There are three ways to work with MyCompiler.

### 1. Run the Automated Tests

From the project root, run:

``` powershell
.\build\Debug\MyCompiler.exe
```

Running the executable without arguments starts the built-in
verification suite.

The current project documentation describes five integration tests
covering:

-   Basic arithmetic and operator precedence.
-   Variable assignment and unary minus.
-   Relational comparisons.
-   Integer division.
-   Compile-time division-by-zero detection.

### 2. Use the CLI

You can inspect and compile a MyLang program from the terminal.

Example:

``` powershell
.\build\Debug\MyCompiler.exe --inspect "int a = 10; int b = 20; print((a + b) * 2);" program.exe
```

The inspection output includes:

-   Lexer token information.
-   Parser and AST information.
-   Semantic analysis and Symbol Table details.
-   Generated LLVM IR.
-   Native executable generation status and runtime output.

The CLI also supports the `--compile` flag.

### 3. Launch the Web IDE

The project includes a browser-based IDE.

#### Start the server

Open a terminal in the project root and run:

``` powershell
python server.py
```

#### Open the IDE

Open your browser and visit:

``` text
http://localhost:8080
```

#### Use the IDE

1.  Open the Web IDE in your browser.
2.  Write a MyLang program in the editor, or select an available sample.
3.  Click **Run** to execute the compiler pipeline.
4.  View the program's output.
5.  Select an individual phase to inspect its result.

------------------------------------------------------------------------

## Understanding the Compiler Phases

The Web IDE provides an individual inspector for each compiler stage.

  -----------------------------------------------------------------------
  Phase                               What you can inspect
  ----------------------------------- -----------------------------------
  **1. Lexer**                        Tokens, token types, lexemes, line
                                      numbers, and column positions

  **2. Parser**                       The Abstract Syntax Tree and its
                                      hierarchical structure

  **3. Semantic Analysis**            Symbol Table, variable information,
                                      and semantic validation

  **4. LLVM IR**                      Generated LLVM instructions,
                                      including memory operations and
                                      arithmetic

  **5. Code Generation**              Target information, Clang
                                      compilation log, executable
                                      generation, and runtime output
  -----------------------------------------------------------------------

This phase-by-phase view helps students understand how source code is
transformed into a native executable.

------------------------------------------------------------------------

## Example Programs

### Example 1 --- Basic Arithmetic

``` c
int a = 10;
int b = 20;
int result = (a + b) * 2;
print(result);
```

Expected output:

``` text
60
```

### Example 2 --- Variable Assignment

``` c
int count = 5;
count = count + 10;
print(count);
```

Expected output:

``` text
15
```

### Example 3 --- Comparison

``` c
int a = 10;
int b = 20;
int result = a < b;
print(result);
```

Expected output:

``` text
1
```

### Example 4 --- Integer Division

``` c
int result = 10 / 3;
print(result);
```

Expected output:

``` text
3
```

### Example 5 --- Division by Zero

``` c
int result = 10 / 0;
print(result);
```

Expected result:

``` text
Semantic Error: Division by zero detected in expression.
```

The semantic analyzer is expected to detect this constant
division-by-zero case before LLVM IR generation.

------------------------------------------------------------------------

## Troubleshooting

### 1. CMake cannot find LLVM

**Possible reason:** `LLVM_DIR` is not set correctly or does not point
to the LLVM CMake package directory.

**Try:**

``` powershell
$env:LLVM_DIR = "C:\LLVM-Dev\lib\cmake\llvm"
cmake -B build -S . -DLLVM_DIR="$env:LLVM_DIR"
```

Confirm that `LLVMConfig.cmake` exists in the selected directory.

### 2. `cl.exe` is not recognized

**Possible reason:** The terminal does not have the MSVC compiler
environment configured.

**Try:** Close the terminal and open **Developer PowerShell for Visual
Studio**, then run the CMake commands again.

### 3. `cmake` is not recognized

**Possible reason:** CMake is not installed or its installation
directory is not available in PATH.

**Try:** Install CMake or add its installation directory to PATH, then
open a new terminal.

### 4. `clang` is not recognized

**Possible reason:** The LLVM installation's `bin` directory is not
available in PATH.

**Try:** Add the LLVM `bin` directory to PATH, open a new terminal, and
check:

``` powershell
clang --version
```

### 5. The Web IDE does not open

**Check the following:**

-   Make sure you are running the command from the project root.
-   Confirm that Python is installed.
-   Check that the server starts without an error.
-   Open `http://localhost:8080` in your browser.
-   Check whether another process is already using port `8080`.

If the problem continues, review the error printed in the terminal where
`server.py` is running.

------------------------------------------------------------------------

## License

This project is licensed under the MIT License. See the
[LICENSE](LICENSE) file for details.
