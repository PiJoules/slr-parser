#include "compiler.h"

lang::Compiler::Compiler(): 
    lexer_(lang::LangLexer(lang::LANG_TOKENS)),
    parser_(parsing::Parser(lexer_, lang::LANG_GRAMMAR)){}

cppnodes::Module* lang::Compiler::compile(std::string code){
    Module* module_node = static_cast<Module*>(parser_.parse(code));
    assert(lexer_.empty());

    cppnodes::Module* cpp_module_node = visit_module(module_node);

    delete module_node;

    return cpp_module_node;
}

/**
 * Just convert the body vectors in each module.
 */
cppnodes::Module* lang::Compiler::visit_module(Module* module){
    std::vector<Node*> body;

    for (ModuleStmt* stmt : module->body()){
    }
}

int main(){
    lang::Compiler compiler;

    std::string code = R"(
def main() -> int:
    return 0
)";

    cppnodes::Module* module = compiler.compile(code);

    std::cerr << module->str() << std::endl;

    delete module;

    return 0;
}
