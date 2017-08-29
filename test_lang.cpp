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
    assert(lexer.token(nullptr).symbol == lang::tokens::NEWLINE);

    // line 2
    assert(lexer.token(nullptr).symbol == "DEF");
    assert(lexer.token(nullptr).symbol == "NAME");
    assert(lexer.token(nullptr).symbol == "LPAR");
    assert(lexer.token(nullptr).symbol == "RPAR");
    assert(lexer.token(nullptr).symbol == "COLON");
    assert(lexer.token(nullptr).symbol == lang::tokens::NEWLINE);

    // line 3
    assert(lexer.token(nullptr).symbol == lang::tokens::INDENT);
    assert(lexer.token(nullptr).symbol == "NAME");
    assert(lexer.token(nullptr).symbol == "ADD");
    assert(lexer.token(nullptr).symbol == "NAME");
    assert(lexer.token(nullptr).symbol == lang::tokens::NEWLINE);
    assert(lexer.token(nullptr).symbol == lang::tokens::DEDENT);

    assert(lexer.empty());
}

void test_regular(){
    const std::string code = R"(
def func():
    x + y
)";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Parser parser(lexer, lang::LANG_GRAMMAR);

    lang::Module* module_node = static_cast<lang::Module*>(parser.parse(code));
    assert(lexer.empty());

    // Check the nodes
    assert(module_node->body().size() == 1);
    lang::FuncDef* func_def = static_cast<lang::FuncDef*>(module_node->body()[0]);
    assert(func_def->suite().size() == 1);

    assert(module_node->str() == "def func() -> int:\n    x + y");

    delete module_node;
}

void test_empty(){
    const std::string code = "";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Parser parser(lexer, lang::LANG_GRAMMAR);
    lang::Module* module_node = static_cast<lang::Module*>(parser.parse(code));

    assert(module_node->body().empty());
    assert(module_node->str() == "");

    delete module_node;
}

void test_fictitios_token(){
    const std::string code = R"(
def func():
    x + -y
)";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Parser parser(lexer, lang::LANG_GRAMMAR);

    lang::Module* module_node = static_cast<lang::Module*>(parser.parse(code));
    assert(lexer.empty());

    assert(module_node->str() == "def func() -> int:\n    x + -y");

    delete module_node;
}

void test_ending_on_func_suite(){
    const std::string code = R"(
def main():
    friends = {"john", "pat", "gary", "michael"}
    for i, name in enumerate(friends):
        print("iteration {} is {}".format(i, name))
)";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Parser parser(lexer, lang::LANG_GRAMMAR);

    lang::Module* module_node = static_cast<lang::Module*>(parser.parse(code));
    assert(lexer.empty());

    delete module_node;
}

int main(){
    assert(lang::LANG_GRAMMAR.conflicts().empty());

    test_tokens();
    test_regular();
    test_empty();
    test_fictitios_token();
    test_ending_on_func_suite();

    return 0;
}
