# Paradox Language

Paradox is a toy procedural language built to illustrate compiler design concepts using LLVM as a backend. It is minimal by design — the goal is to keep the focus on how a compiler works rather than on language complexity.

---

## Language Overview

- **Single Datatype**: All values are 64-bit floating point (`double`).
- **Implicit Typing**: No type declarations needed; every value is a double.
- **Simple Syntax**: Curly-brace blocks, familiar operators, easy to read.

---

## Example

A simple Paradox program to compute the larger of two values:
```paradox
def max(a, b) {
    if (a > b) {
        a;
    } else {
        b;
    }
}

max(10, 3);
```

A loop that counts down to zero:
```paradox
def countdown(n) {
    cycle (n > 0) {
        n = n - 1;
    }
    n;
}

countdown(5);
```

---

## Language Features

### Function Definitions

Functions are declared with `def`, a name, a parameter list, and a curly-brace body.
```paradox
def multiply(a, b) {
    a * b;
}
```

### Conditionals

`if/else` branches on any expression. The else block is optional.
```paradox
def sign(x) {
    if (x > 0) {
        1;
    } else {
        0;
    }
}
```

### Loops

`cycle` repeats its body as long as the condition is non-zero — similar to `while`.
```paradox
def sum(n) {
    total = 0;
    cycle (n > 0) {
        total = total + n;
        n = n - 1;
    }
    total;
}
```

### Binary Operators

| Operator | Meaning |
|----------|---------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |
| `>` | Greater than |
| `<` | Less than |
| `>=` | Greater than or equal |
| `<=` | Less than or equal |

---

## Grammar
```
program              := top-level*
top-level            := function-definition | expression-statement

function-definition  := 'def' identifier '(' params ')' '{' statement* '}'
params               := identifier (',' identifier)*

statement            := expression-statement
                      | if-statement
                      | cycle-statement
                      | assignment-statement

expression-statement := expression ';'
assignment-statement := identifier '=' expression ';'

if-statement         := 'if' '(' expression ')' '{' statement* '}'
                        ['else' '{' statement* '}']

cycle-statement      := 'cycle' '(' expression ')' '{' statement* '}'

expression           := binary-expression
                      | call-expression
                      | number
                      | variable
                      | '(' expression ')'

call-expression      := identifier '(' arguments ')'
arguments            := expression (',' expression)*
binary-expression    := expression operator expression
operator             := '+' | '-' | '*' | '/' | '>' | '<' | '>=' | '<='
```

---

## Project Structure
```
ParadoxCC/
├── main.cpp              # Entry point
├── input.txt             # Source file to compile
├── Grammar.txt           # Language grammar reference
├── Makefile              # Build configuration
├── lexer/
│   ├── lexer.h
│   └── lexer.cpp         # Tokenizer
├── parser/
│   ├── parser.h
│   └── parser.cpp        # Recursive descent parser + AST
└── codegen/
    ├── codegen.h
    └── codegen.cpp       # LLVM IR code generation
```

---

## Building

### Prerequisites

- `clang++` or `g++`
- LLVM (with `llvm-config` available in PATH)

### Build
```bash
make
```

### Clean
```bash
make clean
```

### Manual build
```bash
clang++ main.cpp lexer/lexer.cpp parser/parser.cpp codegen/codegen.cpp \
  $(llvm-config --cxxflags --ldflags --libs core) -std=c++17 -I. -o paradoxCC
```

---

## Usage

1. Write your Paradox source code in `input.txt`
2. Run the compiler:
```bash
./paradoxCC
```
3. Check the outputs:
   - `tokens_generated.txt` — token stream produced by the lexer
   - `IR_generated.txt` — LLVM IR generated from your program

---

