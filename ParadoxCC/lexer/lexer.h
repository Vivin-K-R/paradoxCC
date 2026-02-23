#pragma once
#include <string>
#include <vector>

enum Token{
    tok_eof=-1,
    tok_def=-2,
    tok_extern=-3,
    //tok_paradox=-4,
    tok_if=-5,
    tok_else=-6,
    tok_cycle=-7,
    tok_identifier=-8,
    tok_number=-9,
};

struct TokenInfo{
    Token type;
    std::string txt;
    double numberValue;
};

class Lexer {
    std::string Source;
    size_t Index;
    char LastChar;

public:
    Lexer(const std::string &src);
    std::vector<TokenInfo> makeTokens();

private:
    char getChar();
    TokenInfo getNextToken();
};
