#include "lang.h"

int main(){
    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Grammar grammar(lexer, lang::LANG_RULES, lang::LANG_PRECEDENCE);
    grammar.dump_grammar(std::cout);

    return 0;
}
