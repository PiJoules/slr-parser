#include "lang.h"

/**
 * Test parsing a simple string.
 */
void test_tokens(){
    const std::string code = R"(
def func():
    x + y 
)";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    lexer.input(code);

    // line 1
    assert(lexer.token(nullptr).symbol == "NEWLINE");

    // line 2
    assert(lexer.token(nullptr).symbol == "DEF");
    assert(lexer.token(nullptr).symbol == "NAME");
    assert(lexer.token(nullptr).symbol == "LPAR");
    assert(lexer.token(nullptr).symbol == "RPAR");
    assert(lexer.token(nullptr).symbol == "COLON");
    assert(lexer.token(nullptr).symbol == "NEWLINE");

    // line 3
    assert(lexer.token(nullptr).symbol == lang::tokens::INDENT);
    assert(lexer.token(nullptr).symbol == "NAME");
    assert(lexer.token(nullptr).symbol == "ADD");
    assert(lexer.token(nullptr).symbol == "NAME");
    assert(lexer.token(nullptr).symbol == "NEWLINE");
    assert(lexer.token(nullptr).symbol == lang::tokens::DEDENT);

    assert(lexer.empty());
}

void test_parser(){
    const std::string code = R"(
def func():
    x + y
)";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Parser parser(lexer, lang::LANG_RULES, lang::LANG_PRECEDENCE);
    assert(parser.conflicts().empty());

    lang::Module* module_node = static_cast<lang::Module*>(parser.parse(code));
    assert(lexer.empty());

    // Check the nodes
    assert(module_node->body().size() == 2);  // The first newline and the func def 

    std::cerr << module_node->str();

    delete module_node;
}

int main(){
    test_tokens();
    test_parser();

    return 0;
}
