#include "compiler.h"

lang::Compiler::Compiler(): 
    lexer_(lang::LangLexer(lang::LANG_TOKENS)),
    parser_(parsing::Parser(lexer_, lang::LANG_GRAMMAR)){}

cppnodes::Module* lang::Compiler::compile(std::string code){
    Module* module_node = static_cast<Module*>(parser_.parse(code));
    assert(lexer_.empty());

    std::cerr << 1 << std::endl;

    void* cpp_module_node = module_node->accept(*this);

    std::cerr << 2 << std::endl;

    delete module_node;

    std::cerr << 3 << std::endl;

    return static_cast<cppnodes::Module*>(cpp_module_node);
}

/**
 * Just convert the body vectors in each module.
 */
void* lang::Compiler::visit(Module* module){
    std::vector<Node*> body;

    for (ModuleStmt* stmt : module->body()){
        std::cerr << "module stmt accepting..." << std::endl;
        void* cpp_module_stmt = stmt->accept(*this);
        std::cerr << "module stmt done accepting" << std::endl;
        body.push_back(static_cast<Node*>(cpp_module_stmt));
    }

    cppnodes::Module* cpp_module = new cppnodes::Module(body);
    return cpp_module;
}

void* lang::Compiler::visit(FuncDef* funcdef){
    std::string funcname = funcdef->name();
    std::vector<cppnodes::VarDecl*> cpp_args;
    std::vector<Node*> cpp_body;

    cppnodes::FuncDef* cpp_funcdef = new cppnodes::FuncDef(
            funcname, "int", cpp_args, cpp_body);

    return cpp_funcdef;
}

void* lang::Compiler::visit(ReturnStmt* returnstmt){
    Expr* expr = returnstmt->expr();

    cppnodes::Expr* cpp_expr = static_cast<cppnodes::Expr*>(expr->accept(*this));
    cppnodes::ReturnStmt* cpp_return = new cppnodes::ReturnStmt(cpp_expr);

    return cpp_return;
}

void* lang::Compiler::visit(Int* int_expr){
    cppnodes::Int* cpp_int = new cppnodes::Int(int_expr->value());
    return cpp_int;
}

int main(){
    lang::Compiler compiler;

    std::string code = R"(
def main():
    return 0
)";

    cppnodes::Module* module = compiler.compile(code);

    std::cerr << module->str() << std::endl;

    delete module;

    return 0;
}
