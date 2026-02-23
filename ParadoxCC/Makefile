CXX      = g++
CXXFLAGS = $(shell llvm-config --cxxflags) -std=c++17 -I.
LDFLAGS  = $(shell llvm-config --ldflags --libs core)

SRCS = main.cpp \
       lexer/lexer.cpp \
       parser/parser.cpp \
       codegen/codegen.cpp

TARGET = paradoxCC

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
