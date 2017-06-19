#include "lang.h"

int main(){
    lang::Parser parser(lang::LANG_RULES);
    parser.dump_grammar();

    return 0;
}
