#pragma once

#include "../parser/parser.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <map>
#include <string>

//All LLVM objects (types, constants, functions) are stored inside it. 
// just pass it to everything that needs to create something
extern llvm::LLVMContext TheContext;
// generates the IR
extern llvm::IRBuilder<> Builder;
// holds all our functions. At the end we print this to get your IR.
extern llvm::Module TheModule;

// Symbol table
extern std::map<std::string, llvm::Value*> NamedValues;

// One function per AST node
//Value* is a pointer to the result of any computation in LLVM.
llvm::Value*    codegenNumber   (NumberExprAST*   node);
llvm::Value*    codegenVariable (VariableExprAST* node);
llvm::Value*    codegenBinary   (BinaryExprAST*   node);
llvm::Value*    codegenCall     (CallExprAST*      node);
llvm::Value*    codegenAssign   (AssignExprAST*   node);
llvm::Value*    codegenIf       (IfStmtAST*        node);
llvm::Value*    codegenCycle    (CycleStmtAST*    node);
llvm::Function* codegenPrototype(PrototypeAST*    node);
llvm::Function* codegenFunction (FunctionAST*     node);

//calls the right function based on node type
llvm::Value* codegen(ASTNode* node);
