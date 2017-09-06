#include "cpp_nodes.h"
#include <vector>
#include <cassert>
#include <iostream>

using namespace cppnodes;

void test_func_def(){
    std::shared_ptr<Name> arg(new Name("x"));
    std::shared_ptr<Name> func(new Name("func"));
    assert(func->str() == "func");

    std::vector<std::shared_ptr<Expr>> args = {arg};
    std::shared_ptr<Call> call(new Call(func, args));
    assert(call->str() == "func(x)");

    std::shared_ptr<ReturnStmt> ret_stmt(new ReturnStmt(call));
    assert(ret_stmt->str() == "return func(x);");

    std::vector<std::shared_ptr<parsing::Node>> func_body = {ret_stmt};
    std::shared_ptr<Type> int_type(new Type(std::make_shared<Name>("int")));
    std::shared_ptr<RegVarDecl> var_decl(new RegVarDecl("x", int_type));
    assert(var_decl->str() == "int x");
    std::vector<std::shared_ptr<VarDecl>> args_list = {var_decl};
    std::shared_ptr<FuncDef> funcdef(new FuncDef("main", "int", args_list, func_body));

    std::string full_code = "int main(int x){\n    return func(x);\n}";
    assert(funcdef->str() == full_code);

    std::vector<std::shared_ptr<parsing::Node>> module_body = {funcdef};
    std::shared_ptr<Module> module(new Module(module_body));

    assert(funcdef->str() == full_code);
}

int main(){
    test_func_def();

    return 0;
}
