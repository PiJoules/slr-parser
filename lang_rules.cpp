#include "lang.h"

#define DEFAULT_FUNC_RETURN_TYPE std::make_shared<lang::NameTypeDecl>("int")

/****************** Lexer tokens *****************/

std::unordered_map<std::string, std::string> RESERVED_NAMES = {
    {"def", "DEF"},
    {"return", "RETURN"},
    {"if", "IF"},
    {"for", "FOR"},
    {"in", "IN"},
};

void reserved_name(lexing::LexToken& tok){
    if (RESERVED_NAMES.find(tok.value) != RESERVED_NAMES.end()){
        tok.symbol = RESERVED_NAMES[tok.value];
    }
}

void comment(lexing::LexToken& tok){
    tok.symbol = lexing::tokens::COMMENT;
}

void trim_string_quotes(lexing::LexToken& tok){
    assert(tok.value.size() >= 2);
    tok.value = tok.value.substr(1, tok.value.size()-2);
}

// Tokens are stored in an unordered map and have no iterative order
const lexing::TokensMap lang::LANG_TOKENS = {
    // Values
    {"INT", {R"(\d+)", nullptr}},
    {"NAME", {R"([a-zA-Z_][a-zA-Z0-9_]*)", reserved_name}},
    {"STRING", {R"(\"[^\"\\]*(\\.[^\"\\]*)*\")", trim_string_quotes}},

    // Binary operators
    // Arithmetic
    {"ADD", {R"(\+)", nullptr}},
    {"SUB", {R"(-(?!>))", nullptr}},
    {"MUL", {R"(\*)", nullptr}},
    {"DIV", {R"(\\)", nullptr}},

    // Member access
    {"ARROW", {R"(\-\>)", nullptr}},
    {"DOT", {R"(\.)", nullptr}},

    // Comparison 
    {"EQ", {R"(\=\=)", nullptr}},
    {"NE", {R"(\!\=)", nullptr}},
    {"LT", {R"(<(?!\=))", nullptr}},
    {"GT", {R"(>(?!\=))", nullptr}},
    {"LTE", {R"(\<\=)", nullptr}},
    {"GTE", {R"(\>\=)", nullptr}},

    // Assignment 
    {"ASSIGN", {R"(=(?![=]))", nullptr}},

    // Containers 
    {"LPAR", {R"(\()", nullptr}},
    {"RPAR", {R"(\))", nullptr}},
    {"LBRACE", {R"(\{)", nullptr}},
    {"RBRACE", {R"(\})", nullptr}},

    // Misc 
    {"DEF", {R"(def)", nullptr}},
    {"RETURN", {"return", nullptr}},
    {"IF", {"if", nullptr}},
    {"FOR", {"for", nullptr}},
    {"IN", {"in", nullptr}},
    {"COLON", {R"(\:)", nullptr}},
    {"COMMA", {R"(\,)", nullptr}},

    // Spacing
    {lang::tokens::NEWLINE, {R"(\n+)", nullptr}},
    {lexing::tokens::COMMENT, {R"([ ]*\#[^\n]*)", nullptr}},
    {"WS", {R"([ ]+)", comment}},
    {lang::tokens::INDENT, {lang::tokens::INDENT, nullptr}},
    {lang::tokens::DEDENT, {lang::tokens::DEDENT, nullptr}},

    // Fictitious tokens 
    {"UMINUS", {"", nullptr}},
};

/********** Parser rules ***************/  

// module : module_stmt_list
std::shared_ptr<void> parse_module(std::vector<std::shared_ptr<void>>& args){
    auto module_stmt_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::ModuleStmt>>>(args[0]);
    return std::make_shared<lang::Module>(*module_stmt_list);
}

// module_stmt_list : func_def
std::shared_ptr<void> parse_module_stmt_list(std::vector<std::shared_ptr<void>>& args){
    auto func_def = std::static_pointer_cast<lang::FuncDef>(args[0]);
    std::shared_ptr<std::vector<std::shared_ptr<parsing::Node>>> module_stmt_list(new std::vector<std::shared_ptr<parsing::Node>>);
    module_stmt_list->push_back(func_def);

    return module_stmt_list;
}

// module_stmt_list : NEWLINE
std::shared_ptr<void> parse_module_stmt_list2(std::vector<std::shared_ptr<void>>& args){
    return std::make_shared<std::vector<std::shared_ptr<parsing::Node>>>();
}

// module_stmt_list : module_stmt_list NEWLINE
std::shared_ptr<void> parse_module_stmt_list3(std::vector<std::shared_ptr<void>>& args){
    return args[0];
}

// module_stmt_list : module_stmt_list func_def
std::shared_ptr<void> parse_module_stmt_list4(std::vector<std::shared_ptr<void>>& args){
    auto module_stmt_list = std::static_pointer_cast<std::vector<std::shared_ptr<parsing::Node>>>(args[0]);
    auto func_def = std::static_pointer_cast<lang::FuncDef>(args[1]);

    module_stmt_list->push_back(func_def);

    return module_stmt_list;
}

// var_decl_list : var_decl 
std::shared_ptr<void> parse_var_decl_list_one_arg(std::vector<std::shared_ptr<void>>& args){
    auto var_decl = std::static_pointer_cast<lang::VarDecl>(args[0]);

    std::shared_ptr<std::vector<std::shared_ptr<lang::VarDecl>>> var_decl_list(new std::vector<std::shared_ptr<lang::VarDecl>>);
    var_decl_list->push_back(var_decl);

    return var_decl_list;
}

// var_decl_list : var_decl_list COMMA var_decl 
std::shared_ptr<void> parse_var_decl_list(std::vector<std::shared_ptr<void>>& args){
    auto var_decl_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::VarDecl>>>(args[0]);
    auto var_decl = std::static_pointer_cast<lang::VarDecl>(args[2]);

    var_decl_list->push_back(var_decl);

    return var_decl_list;
}

// var_decl : NAME COLON type_decl 
std::shared_ptr<void> parse_var_decl(std::vector<std::shared_ptr<void>>& args){
    auto name = std::static_pointer_cast<lexing::LexToken>(args[0]);
    auto type_decl = std::static_pointer_cast<lang::TypeDecl>(args[2]);

    auto var_decl = std::make_shared<lang::VarDecl>(name->value, type_decl);

    return var_decl;
}

// var_assign_list : var_assign 
std::shared_ptr<void> parse_var_assign_list_one_assign(std::vector<std::shared_ptr<void>>& args){
    auto var_assign = std::static_pointer_cast<lang::Assign>(args[0]);
    std::shared_ptr<std::vector<std::shared_ptr<lang::Assign>>> var_assign_list(new std::vector<std::shared_ptr<lang::Assign>>);

    var_assign_list->push_back(var_assign);

    return var_assign_list;
}

// var_assign_list : var_assign_list COMMA var_assign 
std::shared_ptr<void> parse_var_assign_list(std::vector<std::shared_ptr<void>>& args){
    auto var_assign_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::Assign>>>(args[0]);
    auto var_assign = std::static_pointer_cast<lang::Assign>(args[2]);

    var_assign_list->push_back(var_assign);

    return var_assign_list;
}

// var_assign : NAME ASSIGN expr
std::shared_ptr<void> parse_var_assign(std::vector<std::shared_ptr<void>>& args){
    auto name = std::static_pointer_cast<lexing::LexToken>(args[0]);
    auto expr = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::Assign>(name->value, expr);
}

// type_decl : NAME 
std::shared_ptr<void> parse_type_decl_name(std::vector<std::shared_ptr<void>>& args){
    auto name = std::static_pointer_cast<lexing::LexToken>(args[0]);
    return std::make_shared<lang::NameTypeDecl>(name->value);
}

// func_def : DEF NAME LPAR RPAR COLON func_suite
std::shared_ptr<void> parse_func_def(std::vector<std::shared_ptr<void>>& args){
    auto name = std::static_pointer_cast<lexing::LexToken>(args[1]);
    auto func_suite = std::static_pointer_cast<std::vector<std::shared_ptr<lang::FuncStmt>>>(args[5]);

    std::shared_ptr<lang::FuncArgs> func_args(new lang::FuncArgs);
    
    auto func_def = std::make_shared<lang::FuncDef>(
            name->value, func_args, DEFAULT_FUNC_RETURN_TYPE, *func_suite);

    return func_def;
}

// func_def : DEF NAME LPAR RPAR ARROW type_decl COLON func_suite
std::shared_ptr<void> parse_func_def_with_return(std::vector<std::shared_ptr<void>>& args){
    auto name = std::static_pointer_cast<lexing::LexToken>(args[1]);
    auto type_decl = std::static_pointer_cast<lang::TypeDecl>(args[5]);
    auto func_suite = std::static_pointer_cast<std::vector<std::shared_ptr<lang::FuncStmt>>>(args[7]);

    std::shared_ptr<lang::FuncArgs> func_args(new lang::FuncArgs);

    return std::make_shared<lang::FuncDef>(name->value, func_args, type_decl, *func_suite);
}

// func_def : DEF NAME LPAR func_args RPAR COLON func_suite 
std::shared_ptr<void> parse_func_def_with_args(std::vector<std::shared_ptr<void>>& args){
    auto name = std::static_pointer_cast<lexing::LexToken>(args[1]);
    auto func_args = std::static_pointer_cast<lang::FuncArgs>(args[3]);
    auto func_suite = std::static_pointer_cast<std::vector<std::shared_ptr<lang::FuncStmt>>>(args[6]);
    
    return std::make_shared<lang::FuncDef>(
            name->value, func_args, DEFAULT_FUNC_RETURN_TYPE, *func_suite);
}

// func_def : DEF NAME LPAR func_args RPAR ARROW type_decl COLON func_suite  
std::shared_ptr<void> parse_func_def_with_args_with_return(std::vector<std::shared_ptr<void>>& args){
    auto name = std::static_pointer_cast<lexing::LexToken>(args[1]);
    auto func_args = std::static_pointer_cast<lang::FuncArgs>(args[3]);
    auto type_decl = std::static_pointer_cast<lang::TypeDecl>(args[6]);
    auto func_suite = std::static_pointer_cast<std::vector<std::shared_ptr<lang::FuncStmt>>>(args[8]);
    
    return std::make_shared<lang::FuncDef>(name->value, func_args, type_decl, *func_suite);
}

// func_args : var_decl_list 
std::shared_ptr<void> parse_arg_list_only_var_decls(std::vector<std::shared_ptr<void>>& args){
    auto var_decl_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::VarDecl>>>(args[0]);
    std::vector<std::shared_ptr<lang::Assign>> kw_args;
    return std::make_shared<lang::FuncArgs>(*var_decl_list, kw_args, false);
}

// func_args : var_assign_list
std::shared_ptr<void> parse_arg_list_only_kwarg_decls(std::vector<std::shared_ptr<void>>& args){
    auto assign_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::Assign>>>(args[0]);
    std::vector<std::shared_ptr<lang::VarDecl>> pos_args;
    return std::make_shared<lang::FuncArgs>(pos_args, *assign_list, false);
}

// func_suite : NEWLINE INDENT func_stmts DEDENT
std::shared_ptr<void> parse_func_suite(std::vector<std::shared_ptr<void>>& args){
    return args[2];
}

// func_stmts : func_stmt 
std::shared_ptr<void> parse_func_stmts(std::vector<std::shared_ptr<void>>& args){
    auto func_stmt = std::static_pointer_cast<parsing::Node>(args[0]);
    std::shared_ptr<std::vector<std::shared_ptr<parsing::Node>>> func_stmts(new std::vector<std::shared_ptr<parsing::Node>>);
    func_stmts->push_back(func_stmt);

    return func_stmts;
}

// func_stmts : func_stmts func_stmt 
std::shared_ptr<void> parse_func_stmts2(std::vector<std::shared_ptr<void>>& args){
    auto func_stmt = std::static_pointer_cast<parsing::Node>(args[1]);
    auto func_stmts = std::static_pointer_cast<std::vector<std::shared_ptr<parsing::Node>>>(args[0]);
    func_stmts->push_back(func_stmt);

    return func_stmts;
}

// func_stmts : func_stmts NEWLINE func_stmt 
std::shared_ptr<void> parse_func_stmts3(std::vector<std::shared_ptr<void>>& args){
    auto func_stmt = std::static_pointer_cast<parsing::Node>(args[2]);
    auto func_stmts = std::static_pointer_cast<std::vector<std::shared_ptr<parsing::Node>>>(args[0]);
    func_stmts->push_back(func_stmt);

    return func_stmts;
}

// func_stmt : simple_func_stmt NEWLINE
std::shared_ptr<void> parse_func_stmt_simple(std::vector<std::shared_ptr<void>>& args){
    return args[0];
}

// func_stmt : compound_func_stmt
std::shared_ptr<void> parse_func_stmt_compound(std::vector<std::shared_ptr<void>>& args){
    return args[0];
}

// simple_func_stmt : expr_stmt
//                  | return_stmt
//                  | var_assign
std::shared_ptr<void> parse_simple_func_stmt(std::vector<std::shared_ptr<void>>& args){
    return args[0];
}

// compound_func_stmt : if_stmt 
//                    | for_loop
std::shared_ptr<void> parse_compound_func_stmt(std::vector<std::shared_ptr<void>>& args){
    return args[0];
}

// expr_stmt : expr 
std::shared_ptr<void> parse_expr_stmt(std::vector<std::shared_ptr<void>>& args){
    auto expr = std::static_pointer_cast<lang::Expr>(args[0]);
    return std::make_shared<lang::ExprStmt>(expr);
}

// return_stmt : RETURN expr  
std::shared_ptr<void> parse_return_stmt(std::vector<std::shared_ptr<void>>& args){
    auto expr = std::static_pointer_cast<lang::Expr>(args[1]);
    return std::make_shared<lang::ReturnStmt>(expr);
}

// if_stmt : IF expr COLON func_suite 
std::shared_ptr<void> parse_if_stmt(std::vector<std::shared_ptr<void>>& args){
    auto expr = std::static_pointer_cast<lang::Expr>(args[1]);
    auto func_suite = std::static_pointer_cast<std::vector<std::shared_ptr<lang::FuncStmt>>>(args[3]);

    return std::make_shared<lang::IfStmt>(expr, *func_suite);
}

// for_loop : FOR expr_list IN expr COLON func_suite 
std::shared_ptr<void> parse_for_loop(std::vector<std::shared_ptr<void>>& args){
    auto expr_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::Expr>>>(args[1]);
    auto expr = std::static_pointer_cast<lang::Expr>(args[3]);
    auto func_suite = std::static_pointer_cast<std::vector<std::shared_ptr<lang::FuncStmt>>>(args[5]);

    return std::make_shared<lang::ForLoop>(*expr_list, expr, *func_suite);
}

// expr : expr DOT NAME
std::shared_ptr<void> parse_member_access(std::vector<std::shared_ptr<void>>& args){
    auto expr = std::static_pointer_cast<lang::Expr>(args[0]);
    auto name = std::static_pointer_cast<lexing::LexToken>(args[2]);

    return std::make_shared<lang::MemberAccess>(expr, name->value);
}

// expr : tuple 
std::shared_ptr<void> parse_tuple_expr(std::vector<std::shared_ptr<void>>& args){
    return args[0];
}

// tuple : LBRACE RBRACE
std::shared_ptr<void> parse_empty_tuple(std::vector<std::shared_ptr<void>>& args){
    return std::make_shared<lang::Tuple>();
}

// tuple : LBRACE expr_list RBRACE
std::shared_ptr<void> parse_tuple(std::vector<std::shared_ptr<void>>& args){
    auto expr_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::Expr>>>(args[1]);
    return std::make_shared<lang::Tuple>(*expr_list);
}

// expr : expr LPAR RPAR 
std::shared_ptr<void> parse_empty_func_call(std::vector<std::shared_ptr<void>>& args){
    auto expr = std::static_pointer_cast<lang::Expr>(args[0]);
    return std::make_shared<lang::Call>(expr);
}

// expr : expr LPAR expr_list RPAR
std::shared_ptr<void> parse_func_call(std::vector<std::shared_ptr<void>>& args){
    auto expr = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::Expr>>>(args[2]);

    return std::make_shared<lang::Call>(expr, *expr_list);
}

// expr_list : expr  
std::shared_ptr<void> parse_call_one_arg(std::vector<std::shared_ptr<void>>& args){
    auto expr = std::static_pointer_cast<lang::Expr>(args[0]);

    std::shared_ptr<std::vector<std::shared_ptr<lang::Expr>>> expr_list(new std::vector<std::shared_ptr<lang::Expr>>);
    expr_list->push_back(expr);

    return expr_list;
}

// expr_list : expr_list COMMA expr
std::shared_ptr<void> parse_expr_list(std::vector<std::shared_ptr<void>>& args){
    auto expr_list = std::static_pointer_cast<std::vector<std::shared_ptr<lang::Expr>>>(args[0]);
    auto expr = std::static_pointer_cast<lang::Expr>(args[2]);

    expr_list->push_back(expr);

    return expr_list;
}

// expr : expr SUB expr 
std::shared_ptr<void> parse_bin_sub_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Sub>(), expr2);
}

// expr : expr ADD expr 
std::shared_ptr<void> parse_bin_add_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Add>(), expr2);
}

// expr : expr MUL expr 
std::shared_ptr<void> parse_bin_mul_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Mul>(), expr2);
}

// expr : expr DIV expr 
std::shared_ptr<void> parse_bin_div_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Div>(), expr2);
}

// expr : expr EQ expr 
std::shared_ptr<void> parse_bin_eq_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Eq>(), expr2);
}

// expr : expr NE expr 
std::shared_ptr<void> parse_bin_ne_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Ne>(), expr2);
}

// expr : expr LT expr 
std::shared_ptr<void> parse_bin_lt_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Lt>(), expr2);
}

// expr : expr GT expr 
std::shared_ptr<void> parse_bin_gt_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Gt>(), expr2);
}

// expr : expr LTE expr 
std::shared_ptr<void> parse_bin_lte_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Lte>(), expr2);
}

// expr : expr GTE expr 
std::shared_ptr<void> parse_bin_gte_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr1 = std::static_pointer_cast<lang::Expr>(args[0]);
    auto expr2 = std::static_pointer_cast<lang::Expr>(args[2]);

    return std::make_shared<lang::BinExpr>(expr1, std::make_shared<lang::Gte>(), expr2);
}

// expr : SUB expr %UMINUS
std::shared_ptr<void> parse_un_sub_expr(std::vector<std::shared_ptr<void>>& args){
    auto expr = std::static_pointer_cast<lang::Expr>(args[1]);
    return std::make_shared<lang::UnaryExpr>(expr, std::make_shared<lang::USub>());
}

// expr : NAME 
std::shared_ptr<void> parse_name_expr(std::vector<std::shared_ptr<void>>& args){
    auto name = std::static_pointer_cast<lexing::LexToken>(args[0]);
    return std::make_shared<lang::NameExpr>(name->value);
}

// expr : INT
std::shared_ptr<void> parse_int_expr(std::vector<std::shared_ptr<void>>& args){
    auto int_tok = std::static_pointer_cast<lexing::LexToken>(args[0]);
    return std::make_shared<lang::Int>(int_tok->value);
}

// expr : STRING
// with the quotes trimmed off
std::shared_ptr<void> parse_string_expr(std::vector<std::shared_ptr<void>>& args){
    auto str = std::static_pointer_cast<lexing::LexToken>(args[0]);
    return std::make_shared<lang::String>(str->value);
}

//std::shared_ptr<void> parse_module_stmt_list_1(std::vector<std::shared_ptr<void>>& args){
//    std::shared_ptr<std::vector<std::shared_ptr<parsing::Node>>> module_stmt_list;
//
//    parsing::Node* module_stmt = std::static_pointer_cast<parsing::Node*>(args[0]);
//
//    module_stmt_list->push_back(module_stmt);
//
//    return module_stmt_list;
//}
//
//std::shared_ptr<void> parse_module_stmt_list_2(std::vector<std::shared_ptr<void>>& args){
//    std::shared_ptr<std::vector<std::shared_ptr<parsing::Node>>> module_stmt_list = std::static_pointer_cast<std::vector<parsing::Node*>*>(args[0]);
//    auto newline = std::static_pointer_cast<lexing::LexToken>(args[1]);
//    parsing::Node* module_stmt = std::static_pointer_cast<parsing::Node*>(args[2]);
//
//    module_stmt_list->push_back(module_stmt);
//
//    delete newline;
//
//    return module_stmt_list;
//}
//
//std::shared_ptr<void> parse_func_def_module_stmt(std::vector<std::shared_ptr<void>>& args){
//    return args[0];
//}

const std::vector<parsing::ParseRule> lang::LANG_RULES = {
    // TODO: See if we can simplify the module_stmt_list into NEWLINE separated module_stmts 
    // like we do with the argument list but with commas
    // Entry point 
    {"module", {"module_stmt_list"}, parse_module},
    //{"module", {lang::tokens::NEWLINE}, parse_module},
    {"module_stmt_list", {"func_def"}, parse_module_stmt_list},
    {"module_stmt_list", {lang::tokens::NEWLINE}, parse_module_stmt_list2},
    {"module_stmt_list", {"module_stmt_list", lang::tokens::NEWLINE}, parse_module_stmt_list3},
    {"module_stmt_list", {"module_stmt_list", "func_def"}, parse_module_stmt_list4},
    //{"module_stmt_list", {"module_stmt"}, parse_module_stmt_list_1},
    //{"module_stmt_list", {"module_stmt_list", lang::tokens::NEWLINE, "module_stmt"}, parse_module_stmt_list_2},
    //{"module_stmt", {"func_def"}, parse_func_def_module_stmt},

    // Functions 
    {"func_def", {"DEF", "NAME", "LPAR", "RPAR", "COLON", "func_suite"}, parse_func_def},
    {"func_def", {"DEF", "NAME", "LPAR", "RPAR", "ARROW", "type_decl", "COLON", "func_suite"}, parse_func_def_with_return},
    {"func_def", {"DEF", "NAME", "LPAR", "func_args", "RPAR", "COLON", "func_suite"}, parse_func_def_with_args},
    {"func_def", {"DEF", "NAME", "LPAR", "func_args", "RPAR", "ARROW", "type_decl", "COLON", "func_suite"}, parse_func_def_with_args_with_return},

    {"func_args", {"var_decl_list"}, parse_arg_list_only_var_decls},
    //{"func_args", {"var_assign_list"}, parse_arg_list_only_kwarg_decls},

    // List of variable declarations
    {"var_decl_list", {"var_decl"}, parse_var_decl_list_one_arg},
    {"var_decl_list", {"var_decl_list", "COMMA", "var_decl"}, parse_var_decl_list},
    {"var_decl", {"NAME", "COLON", "type_decl"}, parse_var_decl},

    // List of variable assignments 
    //{"var_assign_list", {"var_assign"}, parse_var_assign_list_one_assign},
    //{"var_assign_list", {"var_assign_list", "COMMA", "var_assign"}, parse_var_assign_list},
    {"var_assign", {"NAME", "ASSIGN", "expr"}, parse_var_assign},

    // Type declarations
    {"type_decl", {"NAME"}, parse_type_decl_name},

    {"func_suite", {lang::tokens::NEWLINE, lang::tokens::INDENT, "func_stmts", lang::tokens::DEDENT}, parse_func_suite},
    {"func_stmts", {"func_stmt"}, parse_func_stmts},
    {"func_stmts", {"func_stmts", "func_stmt"}, parse_func_stmts2},
    {"func_stmt", {"simple_func_stmt", lang::tokens::NEWLINE}, parse_func_stmt_simple},
    {"func_stmt", {"compound_func_stmt"}, parse_func_stmt_compound},

    {"simple_func_stmt", {"expr_stmt"}, parse_simple_func_stmt},
    {"simple_func_stmt", {"return_stmt"}, parse_simple_func_stmt},
    {"simple_func_stmt", {"var_assign"}, parse_simple_func_stmt},

    {"compound_func_stmt", {"if_stmt"}, parse_compound_func_stmt},
    {"compound_func_stmt", {"for_loop"}, parse_compound_func_stmt},

    // Simple statements - one line 
    {"expr_stmt", {"expr"}, parse_expr_stmt},
    {"return_stmt", {"RETURN", "expr"}, parse_return_stmt},

    // Compound statements - multiple lines 
    // TODO: The rest of the ladder
    {"if_stmt", {"IF", "expr", "COLON", "func_suite"}, parse_if_stmt},
    {"for_loop", {"FOR", "expr_list", "IN", "expr", "COLON", "func_suite"}, parse_for_loop},

    // Member access 
    {"expr", {"expr", "DOT", "NAME"}, parse_member_access},
    //{"expr", {"expr", "ARROW", "expr"}, parse_pointer_member_access},

    // Tuple literal 
    {"expr", {"tuple"}, parse_tuple_expr},
    {"tuple", {"LBRACE", "RBRACE"}, parse_empty_tuple},
    {"tuple", {"LBRACE", "expr_list", "RBRACE"}, parse_tuple},

    // Function calls 
    {"expr", {"expr", "LPAR", "RPAR"}, parse_empty_func_call},
    {"expr", {"expr", "LPAR", "expr_list", "RPAR"}, parse_func_call},
    {"expr_list", {"expr"}, parse_call_one_arg},
    {"expr_list", {"expr_list", "COMMA", "expr"}, parse_expr_list},

    // Binary Expressions
    {"expr", {"expr", "SUB", "expr"}, parse_bin_sub_expr},
    {"expr", {"expr", "ADD", "expr"}, parse_bin_add_expr},
    {"expr", {"expr", "MUL", "expr"}, parse_bin_mul_expr},
    {"expr", {"expr", "DIV", "expr"}, parse_bin_div_expr},

    {"expr", {"expr", "EQ", "expr"}, parse_bin_eq_expr},
    {"expr", {"expr", "NE", "expr"}, parse_bin_ne_expr},
    {"expr", {"expr", "LT", "expr"}, parse_bin_lt_expr},
    {"expr", {"expr", "GT", "expr"}, parse_bin_gt_expr},
    {"expr", {"expr", "LTE", "expr"}, parse_bin_lte_expr},
    {"expr", {"expr", "GTE", "expr"}, parse_bin_gte_expr},

    // Unary expressions
    {"expr", {"SUB", "expr", "%UMINUS"}, parse_un_sub_expr},

    // Atoms
    {"expr", {"NAME"}, parse_name_expr},
    {"expr", {"INT"}, parse_int_expr},
    {"expr", {"STRING"}, parse_string_expr},
};

/**************** Associativity ***************/ 

const parsing::PrecedenceList lang::LANG_PRECEDENCE = {
    // 7
    {parsing::LEFT_ASSOC, {"EQ", "NE"}},

    // 6
    {parsing::LEFT_ASSOC, {"LT", "GT", "LTE", "GTE"}},

    // 4
    {parsing::LEFT_ASSOC, {"ADD", "SUB"}},

    // 3
    {parsing::RIGHT_ASSOC, {"MUL", "DIV"}},

    // 2
    {parsing::RIGHT_ASSOC, {"UMINUS"}},

    // 1

    // Function call, pointer member access, member access
    {parsing::LEFT_ASSOC, {"LPAR", "ARROW", "DOT"}},
};

/**************** Grammar ***************/ 

const parsing::Grammar lang::LANG_GRAMMAR(parsing::keys(lang::LANG_TOKENS),
                                          lang::LANG_RULES,
                                          lang::LANG_PRECEDENCE);
