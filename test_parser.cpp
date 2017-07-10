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
