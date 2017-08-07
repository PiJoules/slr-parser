#include <cassert>
#include <string>

#include "compiler.h"

void test_hello_world(){
    std::string filename = "test_lang_files/hello_world.lang";
    lang::run_lang_file(filename);
}

void test_fib(){
    std::string filename = "test_lang_files/fib.lang";
    lang::run_lang_file(filename);
}

int main(){
    test_hello_world();
    test_fib();

    return 0;
}
