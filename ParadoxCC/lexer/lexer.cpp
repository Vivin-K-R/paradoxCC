#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string &src)
    : Source(src), Index(0), LastChar(' ') {}

std::vector<TokenInfo> Lexer::makeTokens() {
    std::vector<TokenInfo> tokens;
    while (true) {
        TokenInfo tok = getNextToken();
        tokens.push_back(tok);
        if (tok.type == tok_eof)
            break;
    }
    return tokens;
}

char Lexer::getChar() {
    if (Index >= Source.size()) return '\0';
    return Source[Index++];
}

TokenInfo Lexer::getNextToken() {

    while (isspace(LastChar)) LastChar = getChar();

    if (LastChar == '\0') return {tok_eof, "", 0};

    if (isalpha(LastChar)) {
        std::string IdStr;
        do {
            IdStr += LastChar;
            LastChar = getChar();
        } while (isalnum(LastChar));

        if (IdStr == "def") return {tok_def, IdStr, 0};
        if (IdStr == "extern") return {tok_extern, IdStr, 0};
        if (IdStr == "if") return {tok_if, IdStr, 0};
        if (IdStr == "else") return {tok_else, IdStr, 0};
        if (IdStr == "cycle") return {tok_cycle, IdStr, 0};

        return {tok_identifier, IdStr, 0};
    }

    if (isdigit(LastChar) || LastChar == '.') {
        std::string NumStr;
        bool seenDot = false;
        do {
            if (LastChar == '.') {
                if (seenDot) break;
                seenDot = true;
            }
            NumStr += LastChar;
            LastChar = getChar();
        } while (isdigit(LastChar) || LastChar == '.');

        double val = std::stod(NumStr);
        return {tok_number, NumStr, val};
    }

    if (LastChar == '#') {
        do { LastChar = getChar(); }
        while (LastChar != '\0' && LastChar != '\n' && LastChar != '\r');
        if (LastChar != '\0') return getNextToken();
    }

    char ThisChar = LastChar;
    LastChar = getChar();

    return {(Token)ThisChar, std::string(1, ThisChar), 0};
}
