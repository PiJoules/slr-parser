#include "lang.h"

int main(){
    lang::LangLexer lexer(lang::LANG_TOKENS);
    parsing::Grammar grammar(parsing::keys(lexer.tokens()), lang::LANG_RULES, lang::LANG_PRECEDENCE);
    grammar.dump(std::cout);

    return 0;
}
