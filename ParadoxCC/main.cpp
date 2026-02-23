#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "codegen/codegen.h"
#include "llvm/Support/raw_ostream.h"


std::string readContent(const std::string &fname){
    std::ifstream file(fname);
    if(!file){
        std::cerr << "Cannot open "<<fname<<" :/\n";
        exit(1);
    }
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

int main(){
    
    std::string fcontent = readContent("input.txt");

    Lexer lexer(fcontent);
    std::vector<TokenInfo> tokens = lexer.makeTokens();

    // ---- Write Tokens to File ----
    std::ofstream tokenFile("tokens_generated.txt");
    for (const auto &token : tokens) {
        tokenFile << "Token: " << token.txt 
                  << " (" << static_cast<int>(token.type) << ")\n";
    }
    tokenFile.close();

    Parser parse(tokens);
    auto program = parse.parseProgram();
    if(!program){
        std::cerr<<"Parsing failed\n";
        return 1;
    }
    std::cout << "\nParsing completed successfully.\n";

    for(auto& fn : program->Functions)
        codegenFunction(fn.get());

    // ---- Write LLVM IR to File ----
    std::error_code EC;
    llvm::raw_fd_ostream irFile("IR_generated.txt", EC);
    if (EC) {
        std::cerr << "Could not open IR_generated.txt\n";
        return 1;
    }

    TheModule.print(irFile, nullptr);
    irFile.close();

    return 0;
}
