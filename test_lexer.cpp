#include "lang.h"
#include <cassert>

void test_lexer_creation(){
    lang::Lexer lex;
    lang::LexToken tok = lex.token();
    assert(tok.value == "");
    assert(tok.lineno == 1);
    assert(tok.colno == 1);

    // Same output
    tok = lex.token();
    assert(tok.value == "");
    assert(tok.lineno == 1);
    assert(tok.colno == 1);
}

void test_lexer_input(){
    std::string code("x + y\n4-3");
    lang::Lexer lex;
    lex.input(code);

    auto tok = lex.token();
    assert(tok.value == "x");
    assert(tok.lineno == 1);
    assert(tok.colno == 1);

    tok = lex.token();
    assert(tok.value == "+");
    assert(tok.lineno == 1);
    assert(tok.colno == 3);

    tok = lex.token();
    assert(tok.value == "y");
    assert(tok.lineno == 1);
    assert(tok.colno == 5);

    tok = lex.token();
    assert(tok.value == "4");
    assert(tok.lineno == 2);
    assert(tok.colno == 1);

    tok = lex.token();
    assert(tok.value == "-");
    assert(tok.lineno == 2);
    assert(tok.colno == 2);

    tok = lex.token();
    assert(tok.value == "3");
    assert(tok.lineno == 2);
    assert(tok.colno == 3);
}

void test_name(){
    lang::Lexer lex;
    lex.input("_x");
    auto tok = lex.token();
    assert(tok.value == "_x");
    assert(tok.lineno == 1);
    assert(tok.colno == 1);

    lex.input("_92");
    tok = lex.token();
    assert(tok.value == "_92");
    assert(tok.lineno == 1);
    assert(tok.colno == 3);
}

int main(){
    test_lexer_creation();
    test_lexer_input();
    test_name();

    return 0;
}
