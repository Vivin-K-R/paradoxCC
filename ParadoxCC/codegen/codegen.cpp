#include "codegen.h"
#include "../parser/parser.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Type.h"
#include <iostream>

llvm::LLVMContext TheContext;
llvm::IRBuilder<> Builder(TheContext);
llvm::Module TheModule("paradoxCC",TheContext);
std::map<std::string,llvm::Value*> NamedValues;

//creates constant in llvm
llvm::Value* codegenNumber(NumberExprAST* node){
    return llvm::ConstantFP::get(TheContext,llvm::APFloat(node->value));
    /* consider 42 as value
        llvm::APFloat() -> wraps 42 into llvms float type
        get() is static function in constantfp class, it internally handles object creation
    */
}


llvm::Value* codegenVariable(VariableExprAST* node){
    llvm::Value* val = NamedValues[node->name];
    if(!val){
        std::cerr << "Unknown variable : "<<node->name<<"/n";
        return nullptr;
    }
    return val;
}


llvm::Value* codegenBinary(BinaryExprAST* node) {
    llvm::Value* L = codegen(node->lhs.get()); // this get() is to get a raw pointer of lhs node type
    llvm::Value* R = codegen(node->rhs.get());
    if (!L || !R) return nullptr;

    switch (node->op) {
        case '+': return Builder.CreateFAdd(L, R, "addtmp");
        case '-': return Builder.CreateFSub(L, R, "subtmp");
        case '*': return Builder.CreateFMul(L, R, "multmp");
        case '/': return Builder.CreateFDiv(L, R, "divtmp");
        case '<': return Builder.CreateFCmpOLT(L, R, "cmptmp");
        case '>': return Builder.CreateFCmpOGT(L, R, "cmptmp");
        default:
            std::cerr << "Unknown operator\n";
            return nullptr;
    }
}

llvm::Value* codegenCall(CallExprAST* node) {
    llvm::Function* fn = TheModule.getFunction(node->Callee);
    if (!fn) {
        std::cerr << "Unknown function: " << node->Callee << "\n";
        return nullptr;
    }

    std::vector<llvm::Value*> args;
    for (auto& arg : node->Args) {
        llvm::Value* v = codegen(arg.get());
        if (!v) return nullptr;
        args.push_back(v);
    }

    return Builder.CreateCall(fn, args, "calltmp");
}

llvm::Value* codegenAssign(AssignExprAST* node){
    llvm::Value* val = codegen(node->Value.get());
    if(!val) return nullptr;
    NamedValues[node->Name] = val;
    return val;
}

llvm::Function* codegenPrototype(PrototypeAST* node){
    //in codegencall our args are of type value*,
    //here its type*, because in call we just pass values
    //but in prototype we tell the args type!
    //llvm::Type::getDoubleTy(TheContext) -> returns LLVM's representation of the double type
    std::vector<llvm::Type*> doubles(node->Args.size(),llvm::Type::getDoubleTy(TheContext));

    //returns a funtiontype ptr with specif return type and no of args, false tells function i defined are not variadic
    // eg for variadic func : printf("hello"); printf("hello %s", name);      
    llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getDoubleTy(TheContext),doubles,false);

    llvm::Function* fn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, node->Name, TheModule);

    //this is for readability in IR
    //without this vars will be like %0, %1 .. instead of %x, %y
    int i = 0;
    for (auto& arg : fn->args())
        arg.setName(node->Args[i++]);

    return fn;
}

llvm::Function* codegenFunction(FunctionAST* node){
    //check whether func declaration is built or not
    llvm::Function* fn = TheModule.getFunction(node->Proto->getName());
    if(!fn){
        fn = codegenPrototype(node->Proto.get());
    }
    if(!fn) return nullptr;

    //build entry basic block
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(TheContext,"entry",fn);
    Builder.SetInsertPoint(bb);

    NamedValues.clear();
    for(auto& arg : fn->args()){
        NamedValues[std::string(arg.getName())] = &arg;
    }

    //codegen for each statement in func body
    llvm::Value* last = nullptr;
    for(auto& stmt : node->Body){
        last = codegen(stmt.get());
    }

    if(last) Builder.CreateRet(last);
    else Builder.CreateRet(llvm::ConstantFP::get(TheContext, llvm::APFloat(0.0)));

    return fn;
}

llvm::Value* codegenIf(IfStmtAST* node){
    llvm::Value* cond = codegen(node->Condition.get());
    if(!cond) return nullptr;

    llvm::Function* fn = Builder.GetInsertBlock()->getParent();

    //add then block to fn imediately after its creation, thats what fn in below arg tells
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(TheContext,"then",fn);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(TheContext,"else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(TheContext,"merge");

    //branch to blocks based on condition
    Builder.CreateCondBr(cond, thenBB, elseBB);

    //writing into then block
    //direct builder to write into then block
    Builder.SetInsertPoint(thenBB);

    for (auto& stmt : node->Then)
        codegen(stmt.get());
    Builder.CreateBr(mergeBB);

    fn->insert(fn->end(), elseBB);
    Builder.SetInsertPoint(elseBB);
    for (auto& stmt : node->Else)
        codegen(stmt.get());
    Builder.CreateBr(mergeBB);

    fn->insert(fn->end(), mergeBB);
    Builder.SetInsertPoint(mergeBB);

    return llvm::ConstantFP::get(TheContext, llvm::APFloat(0.0));
}

llvm::Value* codegenCycle(CycleStmtAST* node){

    llvm::Function* fn = Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock* condBB  = llvm::BasicBlock::Create(TheContext, "cond", fn);
    llvm::BasicBlock* bodyBB  = llvm::BasicBlock::Create(TheContext, "body");
    llvm::BasicBlock* afterBB = llvm::BasicBlock::Create(TheContext, "after");

    Builder.CreateBr(condBB);

    Builder.SetInsertPoint(condBB);
    llvm::Value* cond = codegen(node->Condition.get());
    if (!cond) return nullptr;
    Builder.CreateCondBr(cond, bodyBB, afterBB);

    fn->insert(fn->end(), bodyBB);
    Builder.SetInsertPoint(bodyBB);
    for (auto& stmt : node->Body)
        codegen(stmt.get());
    Builder.CreateBr(condBB);

    fn->insert(fn->end(), afterBB);
    Builder.SetInsertPoint(afterBB);

    return llvm::ConstantFP::get(TheContext, llvm::APFloat(0.0));
}

llvm::Value* codegen(ASTNode* node) {
    if (auto* n = dynamic_cast<NumberExprAST*>(node))
        return codegenNumber(n);
    if (auto* n = dynamic_cast<VariableExprAST*>(node))
        return codegenVariable(n);
    if (auto* n = dynamic_cast<BinaryExprAST*>(node))
        return codegenBinary(n);
    if (auto* n = dynamic_cast<CallExprAST*>(node))
        return codegenCall(n);
    if (auto* n = dynamic_cast<AssignExprAST*>(node))
        return codegenAssign(n);
    if (auto* n = dynamic_cast<IfStmtAST*>(node))
        return codegenIf(n);
    if (auto* n = dynamic_cast<CycleStmtAST*>(node))
        return codegenCycle(n);

    std::cerr << "Unknown AST node\n";
    return nullptr;
}
