#include<iostream>
//Lexer
//returns 0-255 for unknown character, else
//anyone below
enum Token{
    tok_eof=-1,
    tok_def=-2,
    tok_extern=-3,
    tok_identifier=-4,
    tok_number=-5,
};

static std::string identifierStr;
static double Numval;

static int gettok(){
    static int LastChar=' ';

    //white spaces need to be skipped
    while(isspace(LastChar)){
        LastChar = getchar();
    }

    //identifier
    if(isalpha(LastChar)){
        identifierStr = LastChar;
        while(isalnum(LastChar=getchar())){
            identifierStr += LastChar;
        }
        if(identifierStr == "def") return tok_def;
        if(identifierStr == "extern") return tok_extern;
        return tok_identifier;
    }

    //Numeric values
    if(isdigit(LastChar) || LastChar == '.'){
        std::string NumStr;
        do{
            NumStr += LastChar;
            LastChar = getchar();
        }while(isdigit(LastChar) || LastChar == '.');

        Numval = strtod(NumStr.c_str(),0);
        return tok_number;
    }

    //comments
    if(LastChar == '#'){
        do{
            LastChar = getchar();
        }while(LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        //continue read after comment
        if(LastChar != EOF) return gettok();
    }

    //final checking
    if(LastChar == EOF) return tok_eof;

    //if curr char is not anything in language or we defined, then
    //return the ASCII
    int currChar = LastChar;
    LastChar = getchar();
    return currChar;
}