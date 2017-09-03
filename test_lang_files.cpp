#include <cassert>
#include <string>

#include "compiler.h"

static const std::string lang_files_dir = "example_lang_files/";

void test_hello_world(){
    std::string filename = lang_files_dir + "hello_world.lang";
    lang::run_lang_file(filename);
}

void test_fib(){
    std::string filename = lang_files_dir + "fib.lang";
    lang::run_lang_file(filename);
}

void test_read_input(){
    std::string filename = lang_files_dir + "read_input.lang";
    // TODO: Have this accept stdin to test input()
    lang::compile_lang_file(filename);
}

void test_arguments(){
    std::string filename = lang_files_dir + "arguments.lang";
    lang::run_lang_file(filename);
}

void test_for_loop(){
    std::string filename = lang_files_dir + "for_loop.lang";
    lang::run_lang_file(filename);
}

int main(){
    test_hello_world();
    test_fib();
    test_read_input();
    test_arguments();
    //test_for_loop();

    return 0;
}
