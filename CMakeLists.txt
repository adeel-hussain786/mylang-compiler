cmake_minimum_required(VERSION 3.20)

project(MyCompiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Provide dummy interface targets for optional LLVM compression dependencies if not present on system
if (NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB INTERFACE IMPORTED)
endif()
if (NOT TARGET zstd::libzstd_shared)
    add_library(zstd::libzstd_shared INTERFACE IMPORTED)
endif()
if (NOT TARGET zstd::libzstd_static)
    add_library(zstd::libzstd_static INTERFACE IMPORTED)
endif()

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "LLVM Version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM Directory: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(LLVM_LIBS core support)
message(STATUS "LLVM Libraries: ${LLVM_LIBS}")

# Compiler source files
set(COMPILER_SOURCES
    compiler/main.cpp
    compiler/lexer/Lexer.cpp
    compiler/ast/AST.cpp
    compiler/parser/Parser.cpp
    compiler/semantic/SemanticAnalyzer.cpp
    compiler/llvm/IRGenerator.cpp
    compiler/codegen/CodeGenerator.cpp
)

# Compiler executable
add_executable(MyCompiler ${COMPILER_SOURCES})

# Include path for compiler headers
target_include_directories(MyCompiler PRIVATE compiler)
target_link_libraries(MyCompiler PRIVATE ${LLVM_LIBS})
