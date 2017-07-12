#include "lang.h"

int main(){
    lang::LangLexer lexer(lang::LANG_TOKENS);
    lang::Parser parser(lexer, lang::LANG_RULES, lang::LANG_PRECEDENCE);
    parser.dump_grammar(std::cout);

    return 0;
}
