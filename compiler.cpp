#include "compiler.h"
#include "subprocess.h"

#include <fstream>

lang::Compiler::Compiler(): 
    lexer_(lang::LangLexer(lang::LANG_TOKENS)),
    parser_(parsing::Parser(lexer_, lang::LANG_GRAMMAR)){}

cppnodes::Module* lang::Compiler::compile(std::string code){
    Module* module_node = static_cast<Module*>(parser_.parse(code));
    assert(lexer_.empty());

    void* cpp_module_node = module_node->accept(*this);

    delete module_node;

    return static_cast<cppnodes::Module*>(cpp_module_node);
}

/**
 * Just convert the body vectors in each module.
 */
void* lang::Compiler::visit(Module* module){
    std::vector<Node*> body;

    for (ModuleStmt* stmt : module->body()){
        void* cpp_module_stmt = stmt->accept(*this);
        body.push_back(static_cast<Node*>(cpp_module_stmt));
    }

    cppnodes::Module* cpp_module = new cppnodes::Module(body);
    return cpp_module;
}

void* lang::Compiler::visit(FuncDef* funcdef){
    std::string funcname = funcdef->name();
    std::vector<FuncStmt*> funcsuite = funcdef->suite();
    std::vector<cppnodes::VarDecl*> cpp_args;
    std::vector<Node*> cpp_body;

    for (FuncStmt* stmt : funcsuite){
        void* cpp_stmt = stmt->accept(*this);
        cpp_body.push_back(static_cast<Node*>(cpp_stmt));
    }

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

std::string compile_lang_str(const std::string& code){
    lang::Compiler compiler;
    cppnodes::Module* module = compiler.compile(code);
    std::string cpp_code = module->str();

    delete module;

    return cpp_code;
}

/**
 * Compile a single .cpp source
 */
std::string compile_cpp_file(const std::string& src){
    subprocess::Subprocess subproc;

    std::string compiler = "g++";
    std::string optomization = "-O2";

    std::vector<std::string> cmd = {compiler, optomization, src};

    subprocess::CompletedProcess result = subproc.run(cmd);
    assert(!result.returncode);

    return "a.out";
}

std::string read_file(const std::string& filename){
    std::ifstream f(filename);
    f.seekg(0, std::ios::end);
    size_t size = f.tellg();
    std::string buffer(size, ' ');
    f.seekg(0);
    f.read(&buffer[0], size); 
    f.close();
    return buffer;
}

void write_file(const std::string& filename, const std::string& contents){
    std::ofstream out(filename);
    out << contents;
    out.close();
}

void compile_lang_file(const std::string& src){
    std::string code = read_file(src);
    std::string cpp_code = compile_lang_str(code);
    std::string dest = src + ".cpp";
    write_file(dest, cpp_code);
    compile_cpp_file(dest);
}

int main(int argc, char** argv){
    //subprocess::Subprocess subproc;
    //std::vector<std::string> cmd = {"g++", "test_lang_files/test.lang.cpp"};
    //subprocess::CompletedProcess result = subproc.run(cmd);
    //std::cerr << "stdout: " << result.stdout << std::endl;
    //std::cerr << "stderr: " << result.stderr << std::endl;
    //assert(!result.returncode);

    assert(argc > 1);
    compile_lang_file(argv[1]);

    return 0;
}
