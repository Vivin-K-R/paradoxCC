#pragma once
#include <map>
#include <memory>
#include <vector>
#include <string>
#include "../lexer/lexer.h"

class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class NumberExprAST : public ASTNode {
public:
    double value;
    NumberExprAST(double val) : value(val) {}
};

class VariableExprAST : public ASTNode {
public:
    std::string name;
    VariableExprAST(const std::string &name) : name(name) {}
};

class BinaryExprAST : public ASTNode {
public:
    char op;
    std::unique_ptr<ASTNode> lhs, rhs;
    BinaryExprAST(char op,
                  std::unique_ptr<ASTNode> lhs,
                  std::unique_ptr<ASTNode> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

class CallExprAST : public ASTNode {
public:
    std::string Callee;
    std::vector<std::unique_ptr<ASTNode>> Args;
    CallExprAST(const std::string &Callee,
                std::vector<std::unique_ptr<ASTNode>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
};

class PrototypeAST : public ASTNode {
public:
    std::string Name;
    std::vector<std::string> Args;
    PrototypeAST(const std::string &Name,
                 std::vector<std::string> Args)
        : Name(Name), Args(std::move(Args)) {}
    const std::string &getName() const { return Name; }
};

class FunctionAST : public ASTNode {
public:
    std::unique_ptr<PrototypeAST> Proto;
    std::vector<std::unique_ptr<ASTNode>> Body;
    FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                std::vector<std::unique_ptr<ASTNode>> Body)
        : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

class ProgramAST : public ASTNode {
public:
    std::vector<std::unique_ptr<FunctionAST>> Functions;
    void addFunction(std::unique_ptr<FunctionAST> Fn) {
        Functions.push_back(std::move(Fn));
    }
};

class AssignExprAST : public ASTNode {
public:
    std::string Name;
    std::unique_ptr<ASTNode> Value;
    AssignExprAST(const std::string &Name,
                  std::unique_ptr<ASTNode> Value)
        : Name(Name), Value(std::move(Value)) {}
};

class IfStmtAST : public ASTNode {
public:
    std::unique_ptr<ASTNode> Condition;
    std::vector<std::unique_ptr<ASTNode>> Then;
    std::vector<std::unique_ptr<ASTNode>> Else;
    IfStmtAST(std::unique_ptr<ASTNode> Cond,
              std::vector<std::unique_ptr<ASTNode>> Then,
              std::vector<std::unique_ptr<ASTNode>> Else)
        : Condition(std::move(Cond)),
          Then(std::move(Then)),
          Else(std::move(Else)) {}
};

class CycleStmtAST : public ASTNode {
public:
    std::unique_ptr<ASTNode> Condition;
    std::vector<std::unique_ptr<ASTNode>> Body;
    CycleStmtAST(std::unique_ptr<ASTNode> Cond,
                 std::vector<std::unique_ptr<ASTNode>> Body)
        : Condition(std::move(Cond)),
          Body(std::move(Body)) {}
};


class Parser {
    std::vector<TokenInfo> tokens;
    size_t current;

public:
    Parser(const std::vector<TokenInfo>& toks);
    std::unique_ptr<ProgramAST> parseProgram();

private:
    std::map<char, int> BinOpPrecedence;
    int getTokPrecedence();
    std::unique_ptr<ASTNode> parseBinOpRHS(int exprPrec,
                                           std::unique_ptr<ASTNode> LHS);

    TokenInfo& peek();
    TokenInfo& previous();
    bool isAtEnd();
    void advance();
    bool check(Token type);
    bool match(Token type);

    std::unique_ptr<FunctionAST> parseFunction();
    std::unique_ptr<PrototypeAST> parsePrototype();
    std::vector<std::unique_ptr<ASTNode>> parseBlock();

    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseAssignment();
    std::unique_ptr<ASTNode> parseIfStatement();
    std::unique_ptr<ASTNode> parseCycleStatement();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseIdentifier();
};
