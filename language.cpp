#include "compiler.h"
#include <cassert>

int main(int argc, char** argv){
    assert(argc > 1);
    lang::compile_lang_file(argv[1]);

    return 0;
}
