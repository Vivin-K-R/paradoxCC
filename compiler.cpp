#include <iostream>
#include <memory>
#include <vector>
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

    //final end of file checking
    if(LastChar == EOF) return tok_eof;

    //if curr char is not anything in language or we defined, then
    //return the ASCII
    int currChar = LastChar;
    LastChar = getchar();
    return currChar;
}

//Base class for every expression class (num,var,func)
class ExprAst{
    public:
        virtual ~ExprAst() = default;
};

class NumberExprAst : public ExprAst{
    double val;
    public:
        NumberExprAst(double val) : val(val){}
};

//parameter is a refernce, to avoid extra copy of passing string
class VariableExprAst : public ExprAst{
    std::string Name;
    public:
        VariableExprAst(const std::string &Name) : Name(Name){}
};

//To store expressions, Op for operators, Lhs,Rhs for oprands and sub expressions
class BinaryExprAst : public ExprAst{
    char Op;
    std::unique_ptr<ExprAst>Lhs,Rhs;
    public:
        BinaryExprAst(char Op,std::unique_ptr<ExprAst>Lhs,std::unique_ptr<ExprAst>Rhs) : Op(Op),Lhs(std::move(Lhs)),Rhs(std::move(Rhs)) {}
};

//vector store points of args of function
class CallExprAst : public ExprAst{
    std::string Calle;
    std::vector<std::unique_ptr<ExprAst>> Args;
    public:
        CallExprAst(const std::string &Calle,std::vector<std::unique_ptr<ExprAst>> Args) : Calle(Calle),Args(std::move(Args)) {}
};

//To store function prototype
//getter function return reference and const, caller cannot change string and function itself cannot change name
class PrototypeExprAst : public ExprAst{
    std::string Name;
    std::vector<std::unique_ptr<ExprAst>> Args;
    public:
        PrototypeExprAst(const std::string &Name,std::vector<std::unique_ptr<ExprAst>> Args) : Name(Name),Args(std::move(Args)){}
    const std::string &getName() const {return Name;}
};