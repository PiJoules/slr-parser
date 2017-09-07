#include "compiler.h"
#include "subprocess.h"
#include "utils.h"

#include <fstream>
#include <unordered_map>
#include <unordered_set>


#define NONE_TYPE std::make_shared<lang::NameType>("NoneType")
#define STR_TYPE std::make_shared<lang::NameType>("str")
#define STAR_ARGS_TYPE std::make_shared<lang::StarArgsType>()

static const std::string TUPLE_TYPE_NAME = "LangTuple";
static const std::string STR_TYPE_NAME = "str";

static const std::vector<std::string> LANG_SRCS = {
    "lang_include/lang_io.cpp",
};


lang::LibData lang::create_io_lib(){
    return {
        "lang_io.h",
        {
            {"print", 
                std::shared_ptr<lang::LangType>(
                    new lang::FuncType(NONE_TYPE, {}, true)
                )
            },
            {"input", std::shared_ptr<lang::LangType>(new lang::FuncType(STR_TYPE, {STR_TYPE}, false))},
        },
    };
}

void lang::Compiler::import_builtin_lib(const LibData& lib){
    // Record the library
    include_libs_[lib.lib_filename] = lib;

    // Add all known variables  
    const std::unordered_map<std::string, std::shared_ptr<LangType>>& var_types = lib.lib_var_types;
    Scope& global = global_scope();
    for (auto it = var_types.begin(); it != var_types.end(); ++it){
        global.add_var(it->first, it->second);
    }
}

lang::Compiler::Compiler(): 
    lexer_(lang::LangLexer(lang::LANG_TOKENS)),
    parser_(parsing::Parser(lexer_, lang::LANG_GRAMMAR))
{
    Scope global_scope;
    scope_stack_.push_back(global_scope);

    // Add all builtin libs at start
    import_builtin_lib(create_io_lib());
}

std::shared_ptr<cppnodes::Module> lang::Compiler::compile(std::string code){
    std::shared_ptr<Module> module_node = std::static_pointer_cast<Module>(parser_.parse(code));
    assert(lexer_.empty());

    std::shared_ptr<void> result = std::static_pointer_cast<void>(module_node->accept(*this));
    std::shared_ptr<cppnodes::Module> cpp_module = std::static_pointer_cast<cppnodes::Module>(result);

    // Add any included builtin libs 
    for (auto it = include_libs_.begin(); it != include_libs_.end(); ++it){
        std::string lib = it->first;
        cpp_module->prepend(std::make_shared<cppnodes::Include>(lib));
    }

    return cpp_module;
}

/**
 * Just convert the body vectors in each module.
 */
std::shared_ptr<void> lang::Compiler::visit(Module& module){
    std::vector<std::shared_ptr<parsing::Node>> body;

    for (std::shared_ptr<ModuleStmt> stmt : module.body()){
        std::shared_ptr<void> cpp_module_stmt = stmt->accept(*this);
        body.push_back(std::static_pointer_cast<parsing::Node>(cpp_module_stmt));
    }

    return std::make_shared<cppnodes::Module>(body);
}

std::shared_ptr<lang::FuncType> lang::Compiler::funcdef_type(FuncDef& funcdef){
    std::shared_ptr<TypeDecl> ret_type_decl = funcdef.return_type_decl();
    std::shared_ptr<LangType> ret_type = ret_type_decl->as_type();
    std::vector<std::shared_ptr<LangType>> args;

    std::shared_ptr<FuncArgs> func_args = funcdef.args();

    for (std::shared_ptr<VarDecl> arg : func_args->pos_args()){
        std::shared_ptr<TypeDecl> type_decl = arg->type();
        std::shared_ptr<LangType> type = type_decl->as_type();
        args.push_back(type);
    }

    for (std::shared_ptr<Assign> arg : func_args->keyword_args()){
        std::shared_ptr<Expr> rhs = arg->expr();
        std::shared_ptr<LangType> type = infer(*rhs);
        args.push_back(type);
    }
    
    return std::make_shared<FuncType>(ret_type, args, func_args->has_varargs());
}

std::shared_ptr<void> lang::Compiler::visit(FuncDef& funcdef){
    std::string func_name = funcdef.name();
    std::vector<std::shared_ptr<FuncStmt>> funcsuite = funcdef.suite();
    std::vector<std::shared_ptr<cppnodes::VarDecl>> cpp_args;
    std::vector<std::shared_ptr<parsing::Node>> cpp_body;

    // Add this function to the current scope  
    std::shared_ptr<FuncType> func_type = funcdef_type(funcdef);
    current_scope().add_var(func_name, func_type);

    // Entering a new scope
    enter_scope();

    std::shared_ptr<FuncArgs> func_args = funcdef.args();
    if (!func_args->keyword_args().empty()){
        throw std::runtime_error("Keyword arguments not yet supported.");
    }

    for (std::shared_ptr<VarDecl> decl : func_args->pos_args()){
        // Save the arguments locally
        current_scope().add_var(decl->name(), decl->type()->as_type());

        std::shared_ptr<cppnodes::VarDecl> cpp_decl = std::static_pointer_cast<cppnodes::VarDecl>(decl->accept(*this));
        cpp_args.push_back(cpp_decl);
    }

    for (std::shared_ptr<FuncStmt> stmt : funcsuite){
        std::shared_ptr<void> cpp_stmt = stmt->accept(*this);
        cpp_body.push_back(std::static_pointer_cast<parsing::Node>(cpp_stmt));
    }

    std::shared_ptr<cppnodes::FuncDef> cpp_funcdef(new cppnodes::FuncDef(
            func_name, "int", cpp_args, cpp_body));

    // Exiting scope
    exit_scope();

    return cpp_funcdef;
}

std::shared_ptr<void> lang::Compiler::visit(ReturnStmt& returnstmt){
    std::shared_ptr<Expr> expr = returnstmt.expr();

    std::shared_ptr<cppnodes::Expr> cpp_expr = std::static_pointer_cast<cppnodes::Expr>(expr->accept(*this));
    return std::make_shared<cppnodes::ReturnStmt>(cpp_expr);
}

std::shared_ptr<void> lang::Compiler::visit(VarDecl& var_decl){
    auto cpp_type = std::static_pointer_cast<cppnodes::Type>(visit(*(var_decl.type())));
    return std::make_shared<cppnodes::RegVarDecl>(var_decl.name(), cpp_type);
}

std::shared_ptr<void> lang::Compiler::visit(Assign& assign){
    std::string varname = assign.varname();
    //if (current_scope().has_var(varname)){
    //    // Return cpp assign
    //}
    //else {
    //    // Return var decl
    //}

    std::shared_ptr<Expr> expr = assign.expr();

    std::shared_ptr<LangType> expr_type = infer(*expr);
    std::shared_ptr<TypeDecl> expr_type_decl = expr_type->as_type_decl();

    current_scope().add_var(varname, expr_type);

    std::shared_ptr<cppnodes::RegVarDecl> cpp_var_decl(new cppnodes::RegVarDecl(
                varname, 
                std::static_pointer_cast<cppnodes::Type>(visit(*expr_type_decl))
                ));

    std::shared_ptr<cppnodes::Expr> cpp_expr = std::static_pointer_cast<cppnodes::Expr>(expr->accept(*this));

    return std::make_shared<cppnodes::Assign>(cpp_var_decl, cpp_expr);
}

std::shared_ptr<void> lang::Compiler::visit(IfStmt& if_stmt){
    std::shared_ptr<Expr> cond = if_stmt.cond();
    std::vector<std::shared_ptr<FuncStmt>> body = if_stmt.body();

    std::shared_ptr<cppnodes::Expr> cpp_cond = std::static_pointer_cast<cppnodes::Expr>(cond->accept(*this));

    std::vector<std::shared_ptr<parsing::Node>> cpp_body;
    for (std::shared_ptr<FuncStmt> stmt : body){
        std::shared_ptr<void> cpp_stmt = stmt->accept(*this);
        cpp_body.push_back(std::static_pointer_cast<parsing::Node>(cpp_stmt));
    }

    return std::make_shared<cppnodes::IfStmt>(cpp_cond, cpp_body);
}

/**
 * for target1, target2, ... in expr:
 *     body 
 *
 * for (Type var : expr){
 *     body
 * }
 *
 * Handle the target list using std::tie 
 * https://stackoverflow.com/a/21300447/2775471 
 *
 * Make the range_decl an auto&,
 * immediately declare the variables in the loop body,
 * then use std::tie to unpack.
 */
std::shared_ptr<void> lang::Compiler::visit(ForLoop& for_loop){
    std::string rand_varname = current_scope().rand_varname();
    std::shared_ptr<cppnodes::Name> auto_type(new cppnodes::Name("auto&"));
    std::shared_ptr<cppnodes::Type> tmp_type(new cppnodes::Type(auto_type));
    std::shared_ptr<cppnodes::VarDecl> range_decl(new cppnodes::RegVarDecl(rand_varname, tmp_type));

    auto range_expr = std::static_pointer_cast<cppnodes::Expr>(visit(*(for_loop.container())));
    
    // std::tie
    std::shared_ptr<cppnodes::ScopeResolution> cpp_std_tie(
            new cppnodes::ScopeResolution(std::make_shared<cppnodes::Name>("std"), "tie"));

    std::vector<std::shared_ptr<cppnodes::Expr>> tie_args;
    for (const std::string& target : for_loop.target_list()){
        tie_args.push_back(std::make_shared<cppnodes::Name>(target));
    }

    std::shared_ptr<cppnodes::Call> tie_call(new cppnodes::Call(cpp_std_tie, tie_args));

    std::shared_ptr<cppnodes::AltAssign> unpack(
            new cppnodes::AltAssign(tie_call, std::make_shared<cppnodes::Name>(rand_varname)));

    std::vector<std::shared_ptr<cppnodes::Stmt>> body = {unpack};

    for (std::shared_ptr<FuncStmt> stmt : for_loop.body()){
        auto cpp_stmt = std::static_pointer_cast<cppnodes::Stmt>(visit(*stmt));
        body.push_back(cpp_stmt);
    }

    return std::make_shared<cppnodes::ForEachLoop>(
        range_decl,
        range_expr,
        body
    );
}

std::shared_ptr<void> lang::Compiler::visit(ExprStmt& expr_stmt){
    std::shared_ptr<Expr> expr = expr_stmt.expr();

    std::shared_ptr<cppnodes::Expr> cpp_expr = std::static_pointer_cast<cppnodes::Expr>(expr->accept(*this));
    return std::make_shared<cppnodes::ExprStmt>(cpp_expr);
}

std::shared_ptr<void> lang::Compiler::visit(Call& call){
    std::shared_ptr<Expr> func = call.func();
    auto cpp_func = std::static_pointer_cast<cppnodes::Expr>(visit(*func));

    std::vector<std::shared_ptr<Expr>> args = call.args();
    std::vector<std::shared_ptr<cppnodes::Expr>> cpp_args;
    for (std::shared_ptr<Expr> arg : args){
        std::shared_ptr<cppnodes::Expr> cpp_arg = std::static_pointer_cast<cppnodes::Expr>(arg->accept(*this));
        cpp_args.push_back(cpp_arg);
    }

    return std::make_shared<cppnodes::Call>(cpp_func, cpp_args);
}

std::shared_ptr<void> lang::Compiler::visit(BinExpr& bin_expr){
    std::shared_ptr<Expr> lhs = bin_expr.lhs();
    std::shared_ptr<BinOperator> op = bin_expr.op();
    std::shared_ptr<Expr> rhs = bin_expr.rhs();

    std::shared_ptr<cppnodes::Expr> cpp_lhs = std::static_pointer_cast<cppnodes::Expr>(lhs->accept(*this));
    std::shared_ptr<cppnodes::BinOperator> cpp_op = std::static_pointer_cast<cppnodes::BinOperator>(op->accept(*this));
    std::shared_ptr<cppnodes::Expr> cpp_rhs = std::static_pointer_cast<cppnodes::Expr>(rhs->accept(*this));

    return std::make_shared<cppnodes::BinExpr>(cpp_lhs, cpp_op, cpp_rhs);
}

/**
 * Creates a brace enclosed initializer list.
 */
std::shared_ptr<void> lang::Compiler::visit(Tuple& tuple_expr){
    std::vector<std::shared_ptr<cppnodes::Expr>> cpp_tuple_members;
    for (std::shared_ptr<lang::Expr> tuple_member : tuple_expr.contents()){
        auto cpp_tuple_member = std::static_pointer_cast<cppnodes::Expr>(visit(*tuple_member));
        cpp_tuple_members.push_back(cpp_tuple_member);
    }

    return std::make_shared<cppnodes::BraceEnclosedList>(cpp_tuple_members);
}

std::shared_ptr<void> lang::Compiler::visit(String& str){
    return std::make_shared<cppnodes::String>(str.value());
}

std::shared_ptr<void> lang::Compiler::visit(NameExpr& name){
    current_scope().check_var_exists(name.name());
    return std::make_shared<cppnodes::Name>(name.name());
}

std::shared_ptr<void> lang::Compiler::visit(Int& int_expr){
    return std::make_shared<cppnodes::Int>(int_expr.value());
}

std::shared_ptr<void> lang::Compiler::visit(Add& op){
    return std::make_shared<cppnodes::Add>();
}

std::shared_ptr<void> lang::Compiler::visit(Sub& op){
    return std::make_shared<cppnodes::Sub>();
}

std::shared_ptr<void> lang::Compiler::visit(Mul& op){
    return std::make_shared<cppnodes::Mul>();
}

std::shared_ptr<void> lang::Compiler::visit(Div& op){
    return std::make_shared<cppnodes::Div>();
}

std::shared_ptr<void> lang::Compiler::visit(Eq& op){
    return std::make_shared<cppnodes::Eq>();
}

std::shared_ptr<void> lang::Compiler::visit(Ne& op){
    return std::make_shared<cppnodes::Ne>();
}

std::shared_ptr<void> lang::Compiler::visit(Lt& op){
    return std::make_shared<cppnodes::Lt>();
}

std::shared_ptr<void> lang::Compiler::visit(Gt& op){
    return std::make_shared<cppnodes::Gt>();
}

std::shared_ptr<void> lang::Compiler::visit(Lte& op){
    return std::make_shared<cppnodes::Lte>();
}

std::shared_ptr<void> lang::Compiler::visit(Gte& op){
    return std::make_shared<cppnodes::Gte>();
}

std::shared_ptr<void> lang::Compiler::visit(NameTypeDecl& name_type_decl){
    std::string type_name = name_type_decl.name();
    return std::make_shared<cppnodes::Type>(std::make_shared<cppnodes::Name>(type_name));
}

/**
 * LangTuple<type1, type2, ...>
 */
std::shared_ptr<void> lang::Compiler::visit(TupleTypeDecl& tuple_type_decl){
    std::shared_ptr<cppnodes::Name> base(new cppnodes::Name(TUPLE_TYPE_NAME));

    std::vector<std::shared_ptr<parsing::Node>> template_args;
    for (std::shared_ptr<TypeDecl> arg : tuple_type_decl.contents()){
        auto cpp_arg = std::static_pointer_cast<parsing::Node>(visit(*arg));
        template_args.push_back(cpp_arg);
    }

    return std::make_shared<cppnodes::Type>(base, template_args);
}

std::shared_ptr<void> lang::Compiler::visit(StringTypeDecl& string_type_decl){
    return std::make_shared<cppnodes::Type>(std::make_shared<cppnodes::Name>(STR_TYPE_NAME));
}

std::shared_ptr<lang::LangType> lang::Compiler::infer(Call& call){
    std::shared_ptr<Expr> func = call.func();
    std::shared_ptr<LangType> result = infer(*func);
    std::shared_ptr<FuncType> func_type = std::static_pointer_cast<FuncType>(result);
    return func_type->return_type();
}

std::shared_ptr<lang::LangType> lang::Compiler::infer(NameExpr& name_expr){
    return current_scope().var_type(name_expr.name());
}

std::shared_ptr<lang::LangType> lang::Compiler::infer(Tuple& tuple_expr){
    std::vector<std::shared_ptr<LangType>> content_types;

    for (std::shared_ptr<Expr> expr : tuple_expr.contents()){
        content_types.push_back(infer(*expr));
    }

    return std::make_shared<TupleType>(content_types);
}

std::shared_ptr<lang::LangType> lang::Compiler::infer(String& str_expr){
    return std::make_shared<StringType>();
}

/************ Cmd line interface **************/

std::string compile_lang_str(const std::string& code){
    lang::Compiler compiler;
    std::shared_ptr<cppnodes::Module> module = compiler.compile(code);
    std::string cpp_code = module->str();

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
        compiler, optomization, "-std=" + standard, "-I", "lang_include/", src,
    };
    cmd.insert(cmd.end(), LANG_SRCS.begin(), LANG_SRCS.end());

    subprocess::CompletedProcess result = subproc.run(cmd);

    if (result.returncode){
        std::ostringstream err;
        err << "Error when compiling generated c++ code from lang:" << std::endl << std::endl;
        err << result.stderr;
        throw std::runtime_error(err.str());
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
