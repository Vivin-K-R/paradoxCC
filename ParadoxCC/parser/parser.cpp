#include "parser.h"
#include<iostream>

Parser::Parser(const std::vector<TokenInfo>& toks)
    : tokens(toks), current(0) {

    BinOpPrecedence['<'] = 10;
    BinOpPrecedence['>'] = 10;
    BinOpPrecedence['+'] = 20;
    BinOpPrecedence['-'] = 20;
    BinOpPrecedence['*'] = 40;
    BinOpPrecedence['/'] = 40;
}

int Parser::getTokPrecedence() {
    if (isAtEnd())
        return -1;
    int tok = (int)(unsigned char)peek().type;
    if (tok > 127 || tok < 0)
        return -1;
    int prec = BinOpPrecedence[(char)tok];
    if (prec <= 0)
        return -1;
    return prec;
}

TokenInfo& Parser::peek() {
    return tokens[current];
}

TokenInfo& Parser::previous() {
    return tokens[current - 1];
}

bool Parser::isAtEnd() {
    return current >= tokens.size() || peek().type == tok_eof;
}

void Parser::advance() {
    if (!isAtEnd()) current++;
}

bool Parser::check(Token type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(Token type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

std::unique_ptr<ProgramAST> Parser::parseProgram(){
    auto program = std::make_unique<ProgramAST>();
    while(!isAtEnd()){
        if(check(tok_def)){
            auto fn = parseFunction();
            if(!fn){
                std::cerr << "Failed to parse function\n";
                return nullptr;
            }
            program->addFunction(std::move(fn));
        }
        else{
            std::cerr << "Expected function definition, got: '"
                      << peek().txt << "' (" << (int)peek().type << ")\n";
            return nullptr;
        }
    }
    return program;
}

std::unique_ptr<FunctionAST> Parser::parseFunction(){
    advance();
    auto proto = parsePrototype();
    if(!proto) return nullptr;
    auto body = parseBlock();
    return std::make_unique<FunctionAST>(std::move(proto), std::move(body));
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype(){
    if (!match(tok_identifier)) {
        std::cerr << "Expected function name\n";
        return nullptr;
    }
    std::string name = previous().txt;
    if (!match((Token)'(')) {
        std::cerr << "Expected '('\n";
        return nullptr;
    }
    std::vector<std::string> args;
    if (!check((Token)')')) {
        do {
            if (!match(tok_identifier)) {
                std::cerr << "Expected argument\n";
                return nullptr;
            }
            args.push_back(previous().txt);
        } while (match((Token)','));
    }
    if (!match((Token)')')) {
        std::cerr << "Expected ',' or ')'\n";
        return nullptr;
    }
    return std::make_unique<PrototypeAST>(name, std::move(args));
}

std::vector<std::unique_ptr<ASTNode>> Parser::parseBlock() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    if (!match((Token)'{')) {
        std::cerr << "Expected '{'\n";
        return statements;
    }
    while (!check((Token)'}') && !isAtEnd()) {
        auto stmt = parseStatement();
        if (!stmt)
            return {};
        statements.push_back(std::move(stmt));
    }
    match((Token)'}');
    return statements;
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (check(tok_if))
        return parseIfStatement();

    if (check(tok_cycle))
        return parseCycleStatement();

    // lookahead: identifier followed by '=' is an assignment
    if (check(tok_identifier) && current + 1 < tokens.size()
        && tokens[current + 1].type == (Token)'=')
        return parseAssignment();

    // expression-statement
    auto expr = parseExpression();
    if (!expr) return nullptr;
    if (!match((Token)';')) {
        std::cerr << "Expected ';'\n";
        return nullptr;
    }
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseAssignment() {
    std::string name = peek().txt;
    advance();   // consume identifier
    advance();   // consume '='

    auto value = parseExpression();
    if (!value) return nullptr;

    if (!match((Token)';')) {
        std::cerr << "Expected ';' after assignment\n";
        return nullptr;
    }

    return std::make_unique<AssignExprAST>(name, std::move(value));
}

std::unique_ptr<ASTNode> Parser::parseIfStatement() {
    advance(); // consume 'if'
    if (!match((Token)'(')) {
        std::cerr << "Expected '(' after 'if'\n";
        return nullptr;
    }
    auto cond = parseExpression();
    if (!cond) return nullptr;
    if (!match((Token)')')) {
        std::cerr << "Expected ')' after if condition\n";
        return nullptr;
    }
    auto thenBlock = parseBlock();
    std::vector<std::unique_ptr<ASTNode>> elseBlock;
    if (check(tok_else)) {
        advance(); // consume 'else'
        elseBlock = parseBlock();
    }
    return std::make_unique<IfStmtAST>(
        std::move(cond),
        std::move(thenBlock),
        std::move(elseBlock));
}

std::unique_ptr<ASTNode> Parser::parseCycleStatement() {
    advance(); // consume 'cycle'

    if (!match((Token)'(')) {
        std::cerr << "Expected '(' after 'cycle'\n";
        return nullptr;
    }

    auto cond = parseExpression();
    if (!cond) return nullptr;

    if (!match((Token)')')) {
        std::cerr << "Expected ')' after cycle condition\n";
        return nullptr;
    }

    auto body = parseBlock();

    return std::make_unique<CycleStmtAST>(
        std::move(cond),
        std::move(body));
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    if (check(tok_number)) {
        double val = peek().numberValue;
        advance();
        return std::make_unique<NumberExprAST>(val);
    }
    if (check(tok_identifier))
        return parseIdentifier();
    if (match((Token)'(')) {
        auto expr = parseExpression();
        if (!match((Token)')')) {
            std::cerr << "Expected ')'\n";
            return nullptr;
        }
        return expr;
    }
    std::cerr << "Unknown token in expression: '"
              << peek().txt << "' type=" << (int)peek().type << "\n";
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseIdentifier() {
    std::string name = peek().txt;
    advance();
    if (!match((Token)'(')) {
        return std::make_unique<VariableExprAST>(name);
    }
    std::vector<std::unique_ptr<ASTNode>> args;
    if (!check((Token)')')) {
        do {
            auto arg = parseExpression();
            if (!arg) return nullptr;
            args.push_back(std::move(arg));
        } while (match((Token)','));
    }
    if (!match((Token)')')) {
        std::cerr << "Expected ')'\n";
        return nullptr;
    }
    return std::make_unique<CallExprAST>(name, std::move(args));
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto LHS = parsePrimary();
    if (!LHS) return nullptr;
    return parseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<ASTNode>
Parser::parseBinOpRHS(int exprPrec, std::unique_ptr<ASTNode> LHS) {
    while (true) {
        int tokPrec = getTokPrecedence();
        if (tokPrec < exprPrec)
            return LHS;
        char binOp = peek().type;
        advance();
        auto RHS = parsePrimary();
        if (!RHS) return nullptr;
        int nextPrec = getTokPrecedence();
        if (tokPrec < nextPrec) {
            RHS = parseBinOpRHS(tokPrec + 1, std::move(RHS));
            if (!RHS) return nullptr;
        }
        LHS = std::make_unique<BinaryExprAST>(binOp, std::move(LHS), std::move(RHS));
    }
}
