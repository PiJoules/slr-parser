#include "compiler.h"
#include "subprocess.h"

#include <fstream>
#include <unordered_map>


const std::unordered_map<std::string, std::string> lang::LIB_VARIABLES = {
    {"print", "lang_io.h"},
};

const std::unordered_set<std::string> lang::LIB_SOURCES = {
    "lang_io.cpp",
};


lang::Compiler::Compiler(): 
    lexer_(lang::LangLexer(lang::LANG_TOKENS)),
    parser_(parsing::Parser(lexer_, lang::LANG_GRAMMAR)){}

cppnodes::Module* lang::Compiler::compile(std::string code){
    Module* module_node = static_cast<Module*>(parser_.parse(code));
    assert(lexer_.empty());

    void* result = module_node->accept(*this);
    cppnodes::Module* cpp_module = static_cast<cppnodes::Module*>(result);

    // Add any included libs 
    for (std::string lib : include_libs_){
        cppnodes::Include* include = new cppnodes::Include(lib);
        cpp_module->prepend(include);
    }

    delete module_node;

    return cpp_module;
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

void* lang::Compiler::visit(ExprStmt* expr_stmt){
    Expr* expr = expr_stmt->expr();

    cppnodes::Expr* cpp_expr = static_cast<cppnodes::Expr*>(expr->accept(*this));
    cppnodes::ExprStmt* cpp_expr_stmt = new cppnodes::ExprStmt(cpp_expr);

    return cpp_expr_stmt;
}

void* lang::Compiler::visit(Call* call){
    Expr* func = call->func();
    cppnodes::Expr* cpp_func = static_cast<cppnodes::Expr*>(func->accept(*this));

    std::vector<Expr*> args = call->args();
    std::vector<cppnodes::Expr*> cpp_args;
    for (Expr* arg : args){
        cppnodes::Expr* cpp_arg = static_cast<cppnodes::Expr*>(arg->accept(*this));
        cpp_args.push_back(cpp_arg);
    }

    cppnodes::Call* cpp_call = new cppnodes::Call(cpp_func, cpp_args);
    return cpp_call;
}

void* lang::Compiler::visit(String* str){
    cppnodes::String* cpp_str = new cppnodes::String(str->value());
    return cpp_str;
}

void* lang::Compiler::visit(NameExpr* name){
    cppnodes::Name* cpp_name = new cppnodes::Name(name->name());

    // Check for builtin libraries that should be included based on used expressions  
    auto found_lib = LIB_VARIABLES.find(name->name());
    if (found_lib != LIB_VARIABLES.end()){
        include_libs_.insert(found_lib->second);
    }

    return cpp_name;
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
    std::string standard = "c++11";

    std::vector<std::string> cmd = {
        compiler, optomization, "-std=" + standard, "-I", "lang_include/", "-L", "lang_include/", src
    };
    //cmd.insert(cmd.end(), lang::LIB_SOURCES.begin(), lang::LIB_SOURCES.end());

    subprocess::CompletedProcess result = subproc.run(cmd);

    if (result.returncode){
        throw std::runtime_error(result.stderr);
    }

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
    assert(argc > 1);
    compile_lang_file(argv[1]);

    return 0;
}
