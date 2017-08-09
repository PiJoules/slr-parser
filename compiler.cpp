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
    parser_(parsing::Parser(lexer_, lang::LANG_GRAMMAR))
{
}

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

    for (VarDecl* decl : funcdef->args()){
        cppnodes::VarDecl* cpp_decl = static_cast<cppnodes::VarDecl*>(decl->accept(*this));
        cpp_args.push_back(cpp_decl);
    }

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

void* lang::Compiler::visit(VarDecl* var_decl){
    cached_type_name_ = var_decl->name();
    TypeDecl* type_decl = var_decl->type();
    cppnodes::VarDecl* cpp_var_decl = static_cast<cppnodes::VarDecl*>(type_decl->accept(*this));
    return cpp_var_decl;
}

void* lang::Compiler::visit(Assign* assign){
    std::string varname = assign->varname();
    Expr* expr = assign->expr();

    TypeDecl* expr_type = infer(expr);
    cppnodes::VarDecl* cpp_var_decl = static_cast<cppnodes::VarDecl*>(expr_type->accept(*this));
    cppnodes::Expr* cpp_expr = static_cast<cppnodes::Expr*>(expr->accept(*this));

    cppnodes::Assign* cpp_assign = new cppnodes::Assign(cpp_var_decl, cpp_expr);

    return cpp_assign;
}

void* lang::Compiler::visit(IfStmt* if_stmt){
    Expr* cond = if_stmt->cond();
    std::vector<FuncStmt*> body = if_stmt->body();

    cppnodes::Expr* cpp_cond = static_cast<cppnodes::Expr*>(cond->accept(*this));

    std::vector<Node*> cpp_body;
    for (FuncStmt* stmt : body){
        void* cpp_stmt = stmt->accept(*this);
        cpp_body.push_back(static_cast<Node*>(cpp_stmt));
    }

    cppnodes::IfStmt* cpp_if_stmt = new cppnodes::IfStmt(cpp_cond, cpp_body);

    return cpp_if_stmt;
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

void* lang::Compiler::visit(BinExpr* bin_expr){
    Expr* lhs = bin_expr->lhs();
    BinOperator* op = bin_expr->op();
    Expr* rhs = bin_expr->rhs();

    cppnodes::Expr* cpp_lhs = static_cast<cppnodes::Expr*>(lhs->accept(*this));
    cppnodes::BinOperator* cpp_op = static_cast<cppnodes::BinOperator*>(op->accept(*this));
    cppnodes::Expr* cpp_rhs = static_cast<cppnodes::Expr*>(rhs->accept(*this));

    cppnodes::BinExpr* cpp_bin_expr = new cppnodes::BinExpr(cpp_lhs, cpp_op, cpp_rhs);

    return cpp_bin_expr;
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

void* lang::Compiler::visit(Add* op){
    cppnodes::Add* cpp_op = new cppnodes::Add;
    return cpp_op;
}

void* lang::Compiler::visit(Sub* op){
    cppnodes::Sub* cpp_op = new cppnodes::Sub;
    return cpp_op;
}

void* lang::Compiler::visit(Mul* op){
    cppnodes::Mul* cpp_op = new cppnodes::Mul;
    return cpp_op;
}

void* lang::Compiler::visit(Div* op){
    cppnodes::Div* cpp_op = new cppnodes::Div;
    return cpp_op;
}

void* lang::Compiler::visit(Eq* op){
    cppnodes::Eq* cpp_op = new cppnodes::Eq;
    return cpp_op;
}

void* lang::Compiler::visit(Ne* op){
    cppnodes::Ne* cpp_op = new cppnodes::Ne;
    return cpp_op;
}

void* lang::Compiler::visit(Lt* op){
    cppnodes::Lt* cpp_op = new cppnodes::Lt;
    return cpp_op;
}

void* lang::Compiler::visit(Gt* op){
    cppnodes::Gt* cpp_op = new cppnodes::Gt;
    return cpp_op;
}

void* lang::Compiler::visit(Lte* op){
    cppnodes::Lte* cpp_op = new cppnodes::Lte;
    return cpp_op;
}

void* lang::Compiler::visit(Gte* op){
    cppnodes::Gte* cpp_op = new cppnodes::Gte;
    return cpp_op;
}

void* lang::Compiler::visit(NameTypeDecl* name_type_decl){
    std::string type_name = name_type_decl->name();
    cppnodes::RegVarDecl* cpp_var_decl = new cppnodes::RegVarDecl(cached_type_name_, type_name);
    return cpp_var_decl;
}

/************ Cmd line interface **************/

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
std::string lang::compile_cpp_file(const std::string& src){
    subprocess::Subprocess subproc;

    std::string compiler = "g++";
    std::string optomization = "-O2";
    std::string standard = "c++11";

    std::vector<std::string> cmd = {
        compiler, optomization, "-std=" + standard, "-I", "lang_include/", "-L", "lang_include/", src
    };

    subprocess::CompletedProcess result = subproc.run(cmd);

    if (result.returncode){
        throw std::runtime_error(result.stderr);
    }

    return "a.out";
}

std::string lang::read_file(const std::string& filename){
    std::ifstream f(filename);
    f.seekg(0, std::ios::end);
    size_t size = f.tellg();
    std::string buffer(size, ' ');
    f.seekg(0);
    f.read(&buffer[0], size); 
    f.close();
    return buffer;
}

void lang::write_file(const std::string& filename, const std::string& contents){
    std::ofstream out(filename);
    out << contents;
    out.close();
}

std::string lang::compile_lang_file(const std::string& src){
    std::string code = read_file(src);
    std::string cpp_code = compile_lang_str(code);
    std::string dest = src + ".cpp";
    write_file(dest, cpp_code);
    return compile_cpp_file(dest);
}

void lang::run_lang_file(const std::string& src){
    std::string exe_file = compile_lang_file(src);

    subprocess::Subprocess subproc;
    std::vector<std::string> cmd = {"./" + exe_file};

    subprocess::CompletedProcess result = subproc.run(cmd);

    if (result.returncode){
        throw std::runtime_error(result.stderr);
    }
}
