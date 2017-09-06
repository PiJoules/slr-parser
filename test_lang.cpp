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
    assert(lexer.token().symbol == lang::tokens::NEWLINE);

    // line 2
    assert(lexer.token().symbol == "DEF");
    assert(lexer.token().symbol == "NAME");
    assert(lexer.token().symbol == "LPAR");
    assert(lexer.token().symbol == "RPAR");
    assert(lexer.token().symbol == "COLON");
    assert(lexer.token().symbol == lang::tokens::NEWLINE);

    // line 3
    assert(lexer.token().symbol == lang::tokens::INDENT);
    assert(lexer.token().symbol == "NAME");
    assert(lexer.token().symbol == "ADD");
    assert(lexer.token().symbol == "NAME");
    assert(lexer.token().symbol == lang::tokens::NEWLINE);
    assert(lexer.token().symbol == lang::tokens::DEDENT);

    assert(lexer.empty());
}

void test_regular(){
    const std::string code = R"(
def func():
    x + y
)";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Parser parser(lexer, lang::LANG_GRAMMAR);

    std::shared_ptr<lang::Module> module_node = std::static_pointer_cast<lang::Module>(parser.parse(code));
    assert(lexer.empty());

    // Check the nodes
    assert(module_node->body().size() == 1);
    std::shared_ptr<lang::FuncDef> func_def = std::static_pointer_cast<lang::FuncDef>(module_node->body()[0]);
    assert(func_def->suite().size() == 1);

    assert(module_node->str() == "def func() -> int:\n    x + y");
}

void test_empty(){
    const std::string code = "";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Parser parser(lexer, lang::LANG_GRAMMAR);
    std::shared_ptr<lang::Module> module_node = std::static_pointer_cast<lang::Module>(parser.parse(code));

    assert(module_node->body().empty());
    assert(module_node->str() == "");
}

void test_fictitios_token(){
    const std::string code = R"(
def func():
    x + -y
)";

    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Parser parser(lexer, lang::LANG_GRAMMAR);

    std::shared_ptr<lang::Module> module_node = std::static_pointer_cast<lang::Module>(parser.parse(code));
    assert(lexer.empty());

    assert(module_node->str() == "def func() -> int:\n    x + -y");
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

    parser.parse(code);
    assert(lexer.empty());
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
