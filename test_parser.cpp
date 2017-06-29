#include "lang.h"

/**
 * Test parsing a simple string.
 */
void test_tokens(){
    const std::string code = R"(
def func():
    x + y
)";

    lang::Lexer lexer(lang::LANG_TOKENS);
    lexer.input(code);

    // line 1
    assert(lexer.token().symbol == "NEWLINE");

    // line 2
    assert(lexer.token().symbol == "DEF");
    assert(lexer.token().symbol == "NAME");
    assert(lexer.token().symbol == "LPAR");
    assert(lexer.token().symbol == "RPAR");
    assert(lexer.token().symbol == "COLON");
    assert(lexer.token().symbol == "NEWLINE");

    // line 3
    assert(lexer.token().symbol == lang::tokens::INDENT);
    assert(lexer.token().symbol == "NAME");
    assert(lexer.token().symbol == "ADD");
    assert(lexer.token().symbol == "NAME");
    assert(lexer.token().symbol == "NEWLINE");
    assert(lexer.token().symbol == lang::tokens::DEDENT);

    assert(lexer.empty());
}

void test_parser(){
    const std::string code = R"(
def func():
    x + y
)";

    lang::Lexer lexer(lang::LANG_TOKENS);
    lang::Parser parser(lexer, lang::LANG_RULES, lang::LANG_PRECEDENCE);
    parser.parse(code);
    assert(lexer.empty());
}

int main(){
    test_tokens();
    test_parser();

    return 0;
}
