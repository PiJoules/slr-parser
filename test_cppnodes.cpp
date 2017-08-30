#include "cpp_nodes.h"
#include <vector>
#include <cassert>
#include <iostream>

using namespace cppnodes;

void test_func_def(){
    Name* arg = new Name("x");
    Name* func = new Name("func");
    assert(func->str() == "func");

    std::vector<Expr*> args = {arg};
    Call* call = new Call(func, args);
    assert(call->str() == "func(x)");

    ReturnStmt* ret_stmt = new ReturnStmt(call);
    assert(ret_stmt->str() == "return func(x);");

    std::vector<lang::Node*> func_body = {ret_stmt};
    RegVarDecl* var_decl = new RegVarDecl("x", new NameType("int"));
    assert(var_decl->str() == "int x");
    std::vector<VarDecl*> args_list = {var_decl};
    FuncDef* funcdef = new FuncDef("main", "int", args_list, func_body);

    std::string full_code = "int main(int x){\n    return func(x);\n}";
    assert(funcdef->str() == full_code);

    std::vector<lang::Node*> module_body = {funcdef};
    Module* module = new Module(module_body);

    assert(funcdef->str() == full_code);
    delete module;
}

int main(){
    test_func_def();

    return 0;
}
