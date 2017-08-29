#include "lang.h"

#define DEFAULT_FUNC_RETURN_TYPE new lang::NameTypeDecl("int")

/****************** Lexer tokens *****************/

std::unordered_map<std::string, std::string> RESERVED_NAMES = {
    {"def", "DEF"},
    {"return", "RETURN"},
    {"if", "IF"},
    {"for", "FOR"},
    {"in", "IN"},
};

void reserved_name(lexing::LexToken& tok, void* data){
    if (RESERVED_NAMES.find(tok.value) != RESERVED_NAMES.end()){
        tok.symbol = RESERVED_NAMES[tok.value];
    }
}

void comment(lexing::LexToken& tok, void* data){
    tok.symbol = lexing::tokens::COMMENT;
}

void trim_string_quotes(lexing::LexToken& tok, void* data){
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
void* parse_module(std::vector<void*>& args, void* data){
    std::vector<lang::ModuleStmt*>* module_stmt_list = static_cast<std::vector<lang::ModuleStmt*>*>(args[0]);

    lang::Module* module = new lang::Module(*module_stmt_list);

    delete module_stmt_list;

    return module;
}

// module_stmt_list : func_def
void* parse_module_stmt_list(std::vector<void*>& args, void* data){
    lang::FuncDef* func_def = static_cast<lang::FuncDef*>(args[0]);
    std::vector<lang::Node*>* module_stmt_list = new std::vector<lang::Node*>;
    module_stmt_list->push_back(func_def);

    return module_stmt_list;
}

// module_stmt_list : NEWLINE
void* parse_module_stmt_list2(std::vector<void*>& args, void* data){
    lexing::LexToken* newline = static_cast<lexing::LexToken*>(args[0]);
    std::vector<lang::Node*>* module_stmt_list = new std::vector<lang::Node*>;

    delete newline;

    return module_stmt_list;
}

// module_stmt_list : module_stmt_list NEWLINE
void* parse_module_stmt_list3(std::vector<void*>& args, void* data){
    lexing::LexToken* newline = static_cast<lexing::LexToken*>(args[1]);
    std::vector<lang::Node*>* module_stmt_list = static_cast<std::vector<lang::Node*>*>(args[0]);

    delete newline;

    return module_stmt_list;
}

// module_stmt_list : module_stmt_list func_def
void* parse_module_stmt_list4(std::vector<void*>& args, void* data){
    lang::FuncDef* func_def = static_cast<lang::FuncDef*>(args[1]);
    std::vector<lang::Node*>* module_stmt_list = static_cast<std::vector<lang::Node*>*>(args[0]);

    module_stmt_list->push_back(func_def);

    return module_stmt_list;
}

// var_decl_list : var_decl 
void* parse_var_decl_list_one_arg(std::vector<void*>& args, void* data){
    lang::VarDecl* var_decl = static_cast<lang::VarDecl*>(args[0]);
    std::vector<lang::VarDecl*>* var_decl_list = new std::vector<lang::VarDecl*>;

    var_decl_list->push_back(var_decl);

    return var_decl_list;
}

// var_decl_list : var_decl_list COMMA var_decl 
void* parse_var_decl_list(std::vector<void*>& args, void* data){
    std::vector<lang::VarDecl*>* var_decl_list = static_cast<std::vector<lang::VarDecl*>*>(args[0]);
    lexing::LexToken* comma = static_cast<lexing::LexToken*>(args[1]);
    lang::VarDecl* var_decl = static_cast<lang::VarDecl*>(args[2]);

    delete comma;

    var_decl_list->push_back(var_decl);

    return var_decl_list;
}

// var_decl : NAME COLON type_decl 
void* parse_var_decl(std::vector<void*>& args, void* data){
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* comma = static_cast<lexing::LexToken*>(args[1]);
    lang::TypeDecl* type_decl = static_cast<lang::TypeDecl*>(args[2]);

    lang::VarDecl* var_decl = new lang::VarDecl(name->value, type_decl);

    delete name;
    delete comma;

    return var_decl;
}

// var_assign_list : var_assign 
void* parse_var_assign_list_one_assign(std::vector<void*>& args, void* data){
    lang::Assign* var_assign = static_cast<lang::Assign*>(args[0]);
    std::vector<lang::Assign*>* var_assign_list = new std::vector<lang::Assign*>;

    var_assign_list->push_back(var_assign);

    return var_assign_list;
}

// var_assign_list : var_assign_list COMMA var_assign 
void* parse_var_assign_list(std::vector<void*>& args, void* data){
    std::vector<lang::Assign*>* var_assign_list = static_cast<std::vector<lang::Assign*>*>(args[0]);
    lexing::LexToken* comma = static_cast<lexing::LexToken*>(args[1]);
    lang::Assign* var_assign = static_cast<lang::Assign*>(args[2]);

    var_assign_list->push_back(var_assign);

    delete comma;

    return var_assign_list;
}

// var_assign : NAME ASSIGN expr
void* parse_var_assign(std::vector<void*>& args, void* data){
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* assign = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr = static_cast<lang::Expr*>(args[2]);

    lang::Assign* var_assign = new lang::Assign(name->value, expr);

    delete name;
    delete assign;

    return var_assign;
}

// type_decl : NAME 
void* parse_type_decl_name(std::vector<void*>& args, void* data){
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[0]);

    lang::TypeDecl* type_decl = new lang::NameTypeDecl(name->value);

    delete name;

    return type_decl;
}

// func_def : DEF NAME LPAR RPAR COLON func_suite
void* parse_func_def(std::vector<void*>& args, void* data){
    lexing::LexToken* def = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[1]);
    lexing::LexToken* lpar = static_cast<lexing::LexToken*>(args[2]);
    lexing::LexToken* rpar = static_cast<lexing::LexToken*>(args[3]);
    lexing::LexToken* colon = static_cast<lexing::LexToken*>(args[4]);
    std::vector<lang::FuncStmt*>* func_suite = static_cast<std::vector<lang::FuncStmt*>*>(args[5]);

    lang::FuncArgs* func_args = new lang::FuncArgs;
    lang::TypeDecl* return_type = DEFAULT_FUNC_RETURN_TYPE;
    
    lang::FuncDef* func_def = new lang::FuncDef(name->value, func_args, return_type, *func_suite);

    delete def;
    delete name;
    delete lpar;
    delete rpar;
    delete colon;
    delete func_suite;

    return func_def;
}

// func_def : DEF NAME LPAR RPAR ARROW type_decl COLON func_suite
void* parse_func_def_with_return(std::vector<void*>& args, void* data){
    lexing::LexToken* def = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[1]);
    lexing::LexToken* lpar = static_cast<lexing::LexToken*>(args[2]);
    lexing::LexToken* rpar = static_cast<lexing::LexToken*>(args[3]);
    lexing::LexToken* arrow = static_cast<lexing::LexToken*>(args[4]);
    lang::TypeDecl* type_decl = static_cast<lang::TypeDecl*>(args[5]);
    lexing::LexToken* colon = static_cast<lexing::LexToken*>(args[6]);
    std::vector<lang::FuncStmt*>* func_suite = static_cast<std::vector<lang::FuncStmt*>*>(args[7]);

    lang::FuncArgs* func_args = new lang::FuncArgs;

    lang::FuncDef* func_def = new lang::FuncDef(name->value, func_args, type_decl, *func_suite);

    delete def;
    delete name;
    delete lpar;
    delete rpar;
    delete arrow;
    delete colon;
    delete func_suite;

    return func_def;
}

// func_def : DEF NAME LPAR func_args RPAR COLON func_suite 
void* parse_func_def_with_args(std::vector<void*>& args, void* data){
    lexing::LexToken* def = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[1]);
    lexing::LexToken* lpar = static_cast<lexing::LexToken*>(args[2]);
    lang::FuncArgs* func_args = static_cast<lang::FuncArgs*>(args[3]);
    lexing::LexToken* rpar = static_cast<lexing::LexToken*>(args[4]);
    lexing::LexToken* colon = static_cast<lexing::LexToken*>(args[5]);
    std::vector<lang::FuncStmt*>* func_suite = static_cast<std::vector<lang::FuncStmt*>*>(args[6]);
    
    lang::TypeDecl* return_type = DEFAULT_FUNC_RETURN_TYPE;
    lang::FuncDef* func_def = new lang::FuncDef(name->value, func_args, return_type, *func_suite);

    delete def;
    delete name;
    delete lpar;
    delete rpar;
    delete colon;
    delete func_suite;

    return func_def;
}

// func_def : DEF NAME LPAR func_args RPAR ARROW type_decl COLON func_suite  
void* parse_func_def_with_args_with_return(std::vector<void*>& args, void* data){
    lexing::LexToken* def = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[1]);
    lexing::LexToken* lpar = static_cast<lexing::LexToken*>(args[2]);
    lang::FuncArgs* func_args = static_cast<lang::FuncArgs*>(args[3]);
    lexing::LexToken* rpar = static_cast<lexing::LexToken*>(args[4]);
    lexing::LexToken* arrow = static_cast<lexing::LexToken*>(args[5]);
    lang::TypeDecl* type_decl = static_cast<lang::TypeDecl*>(args[6]);
    lexing::LexToken* colon = static_cast<lexing::LexToken*>(args[7]);
    std::vector<lang::FuncStmt*>* func_suite = static_cast<std::vector<lang::FuncStmt*>*>(args[8]);
    
    lang::FuncDef* func_def = new lang::FuncDef(name->value, func_args, type_decl, *func_suite);

    delete def;
    delete name;
    delete lpar;
    delete rpar;
    delete arrow;
    delete colon;
    delete func_suite;

    return func_def;
}

// func_args : var_decl_list 
void* parse_arg_list_only_var_decls(std::vector<void*>& args, void* data){
    std::vector<lang::VarDecl*>* var_decl_list = static_cast<std::vector<lang::VarDecl*>*>(args[0]);

    lang::FuncArgs* func_args = new lang::FuncArgs(*var_decl_list, {}, false);

    delete var_decl_list;

    return func_args;
}

// func_args : var_assign_list
void* parse_arg_list_only_kwarg_decls(std::vector<void*>& args, void* data){
    std::vector<lang::Assign*>* assign_list = static_cast<std::vector<lang::Assign*>*>(args[0]);

    lang::FuncArgs* func_args = new lang::FuncArgs({}, *assign_list, false);

    delete assign_list;

    return func_args;
}

// func_suite : NEWLINE INDENT func_stmts DEDENT
void* parse_func_suite(std::vector<void*>& args, void* data){
    lexing::LexToken* newline = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* indent = static_cast<lexing::LexToken*>(args[1]);
    std::vector<lang::Node*>* func_stmts = static_cast<std::vector<lang::Node*>*>(args[2]);
    lexing::LexToken* dedent = static_cast<lexing::LexToken*>(args[3]);

    delete newline;
    delete indent;
    delete dedent;

    return func_stmts;
}

// func_stmts : func_stmt 
void* parse_func_stmts(std::vector<void*>& args, void* data){
    lang::Node* func_stmt = static_cast<lang::Node*>(args[0]);
    std::vector<lang::Node*>* func_stmts = new std::vector<lang::Node*>;
    func_stmts->push_back(func_stmt);

    return func_stmts;
}

// func_stmts : func_stmts func_stmt 
void* parse_func_stmts2(std::vector<void*>& args, void* data){
    lang::Node* func_stmt = static_cast<lang::Node*>(args[1]);
    std::vector<lang::Node*>* func_stmts = static_cast<std::vector<lang::Node*>*>(args[0]);
    func_stmts->push_back(func_stmt);

    return func_stmts;
}

// func_stmts : func_stmts NEWLINE func_stmt 
void* parse_func_stmts3(std::vector<void*>& args, void* data){
    lang::Node* func_stmt = static_cast<lang::Node*>(args[2]);
    lexing::LexToken* newline = static_cast<lexing::LexToken*>(args[1]);
    std::vector<lang::Node*>* func_stmts = static_cast<std::vector<lang::Node*>*>(args[0]);
    func_stmts->push_back(func_stmt);

    delete newline;

    return func_stmts;
}

// func_stmt : simple_func_stmt NEWLINE
void* parse_func_stmt_simple(std::vector<void*>& args, void* data){
    lang::SimpleFuncStmt* simple_func_stmt = static_cast<lang::SimpleFuncStmt*>(args[0]);
    lexing::LexToken* newline = static_cast<lexing::LexToken*>(args[1]);

    delete newline;

    return simple_func_stmt;
}

// func_stmt : compound_func_stmt
void* parse_func_stmt_compound(std::vector<void*>& args, void* data){
    return args[0];
}

// simple_func_stmt : expr_stmt
//                  | return_stmt
//                  | var_assign
void* parse_simple_func_stmt(std::vector<void*>& args, void* data){
    return args[0];
}

// compound_func_stmt : if_stmt 
//                    | for_loop
void* parse_compound_func_stmt(std::vector<void*>& args, void* data){
    return args[0];
}

// expr_stmt : expr 
void* parse_expr_stmt(std::vector<void*>& args, void* data){
    lang::Expr* expr = static_cast<lang::Expr*>(args[0]);
    lang::ExprStmt* expr_stmt = new lang::ExprStmt(expr);
    
    return expr_stmt;
}

// return_stmt : RETURN expr  
void* parse_return_stmt(std::vector<void*>& args, void* data){
    lexing::LexToken* return_token = static_cast<lexing::LexToken*>(args[0]);
    lang::Expr* expr = static_cast<lang::Expr*>(args[1]);

    delete return_token;

    lang::ReturnStmt* return_stmt = new lang::ReturnStmt(expr);

    return return_stmt;
}

// if_stmt : IF expr COLON func_suite 
void* parse_if_stmt(std::vector<void*>& args, void* data){
    lexing::LexToken* if_tok = static_cast<lexing::LexToken*>(args[0]);
    lang::Expr* expr = static_cast<lang::Expr*>(args[1]);
    lexing::LexToken* colon = static_cast<lexing::LexToken*>(args[2]);
    std::vector<lang::FuncStmt*>* func_suite = static_cast<std::vector<lang::FuncStmt*>*>(args[3]);

    lang::IfStmt* if_stmt = new lang::IfStmt(expr, *func_suite);

    delete if_tok;
    delete colon;
    delete func_suite;

    return if_stmt;
}

// for_loop : FOR expr_list IN expr COLON func_suite 
void* parse_for_loop(std::vector<void*>& args, void* data){
    lexing::LexToken* for_tok = static_cast<lexing::LexToken*>(args[0]);
    std::vector<lang::Expr*>* expr_list = static_cast<std::vector<lang::Expr*>*>(args[1]);
    lexing::LexToken* in_tok = static_cast<lexing::LexToken*>(args[2]);
    lang::Expr* expr = static_cast<lang::Expr*>(args[3]);
    lexing::LexToken* colon = static_cast<lexing::LexToken*>(args[4]);
    std::vector<lang::FuncStmt*>* func_suite = static_cast<std::vector<lang::FuncStmt*>*>(args[5]);

    lang::ForLoop* for_loop = new lang::ForLoop(*expr_list, expr, *func_suite);

    delete for_tok;
    delete expr_list;
    delete in_tok;
    delete colon;
    delete func_suite;

    return for_loop;
}

// expr : expr DOT NAME
void* parse_member_access(std::vector<void*>& args, void* data){
    lang::Expr* expr = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* dot = static_cast<lexing::LexToken*>(args[1]);
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[2]);

    lang::MemberAccess* member_access = new lang::MemberAccess(expr, name->value);

    delete dot;
    delete name;

    return member_access;
}

// expr : tuple 
void* parse_tuple_expr(std::vector<void*>& args, void* data){
    return args[0];
}

// tuple : LBRACE RBRACE
void* parse_empty_tuple(std::vector<void*>& args, void* data){
    lexing::LexToken* lbrace = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* rbrace = static_cast<lexing::LexToken*>(args[1]);

    lang::Tuple* tuple = new lang::Tuple;

    delete lbrace;
    delete rbrace;

    return tuple;
}

// tuple : LBRACE expr_list RBRACE
void* parse_tuple(std::vector<void*>& args, void* data){
    lexing::LexToken* lbrace = static_cast<lexing::LexToken*>(args[0]);
    std::vector<lang::Expr*>* expr_list = static_cast<std::vector<lang::Expr*>*>(args[1]);
    lexing::LexToken* rbrace = static_cast<lexing::LexToken*>(args[2]);

    lang::Tuple* tuple = new lang::Tuple(*expr_list);

    delete lbrace;
    delete expr_list;
    delete rbrace;

    return tuple;
}

// expr : expr LPAR RPAR 
void* parse_empty_func_call(std::vector<void*>& args, void* data){
    lang::Expr* expr = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* lpar = static_cast<lexing::LexToken*>(args[1]);
    lexing::LexToken* rpar = static_cast<lexing::LexToken*>(args[2]);

    delete lpar;
    delete rpar;

    lang::Call* call = new lang::Call(expr);

    return call;
}

// expr : expr LPAR expr_list RPAR
void* parse_func_call(std::vector<void*>& args, void* data){
    lang::Expr* expr = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* lpar = static_cast<lexing::LexToken*>(args[1]);
    std::vector<lang::Expr*>* expr_list = static_cast<std::vector<lang::Expr*>*>(args[2]);
    lexing::LexToken* rpar = static_cast<lexing::LexToken*>(args[3]);

    delete lpar;
    delete rpar;

    lang::Call* call = new lang::Call(expr, *expr_list);

    delete expr_list;

    return call;
}

// expr_list : expr  
void* parse_call_one_arg(std::vector<void*>& args, void* data){
    lang::Expr* expr = static_cast<lang::Expr*>(args[0]);

    std::vector<lang::Expr*>* expr_list = new std::vector<lang::Expr*>;
    expr_list->push_back(expr);

    return expr_list;
}

// expr_list : expr_list COMMA expr
void* parse_expr_list(std::vector<void*>& args, void* data){
    std::vector<lang::Expr*>* expr_list = static_cast<std::vector<lang::Expr*>*>(args[0]);
    lexing::LexToken* comma = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr = static_cast<lang::Expr*>(args[2]);

    delete comma;

    expr_list->push_back(expr);

    return expr_list;
}

// expr : expr SUB expr 
void* parse_bin_sub_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Sub* op = new lang::Sub;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr ADD expr 
void* parse_bin_add_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Add* op = new lang::Add;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr MUL expr 
void* parse_bin_mul_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Mul* op = new lang::Mul;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr DIV expr 
void* parse_bin_div_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Div* op = new lang::Div;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr EQ expr 
void* parse_bin_eq_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Eq* op = new lang::Eq;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr NE expr 
void* parse_bin_ne_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Ne* op = new lang::Ne;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr LT expr 
void* parse_bin_lt_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Lt* op = new lang::Lt;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr GT expr 
void* parse_bin_gt_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Gt* op = new lang::Gt;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr LTE expr 
void* parse_bin_lte_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Lte* op = new lang::Lte;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr GTE expr 
void* parse_bin_gte_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* op_tok = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Gte* op = new lang::Gte;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : SUB expr %UMINUS
void* parse_un_sub_expr(std::vector<void*>& args, void* data){
    lexing::LexToken* sub = static_cast<lexing::LexToken*>(args[0]);
    lang::Expr* expr = static_cast<lang::Expr*>(args[1]);

    lang::USub* op = new lang::USub;
    lang::UnaryExpr* unary_expr = new lang::UnaryExpr(expr, op);

    delete sub;

    return unary_expr;
}

// expr : NAME 
void* parse_name_expr(std::vector<void*>& args, void* data){
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[0]);
    lang::NameExpr* expr = new lang::NameExpr(name->value);

    delete name;

    return expr;
}

// expr : INT
void* parse_int_expr(std::vector<void*>& args, void* data){
    lexing::LexToken* int_tok = static_cast<lexing::LexToken*>(args[0]);
    lang::Int* expr = new lang::Int(int_tok->value);
    
    delete int_tok;

    return expr;
}

// expr : STRING
// with the quotes trimmed off
void* parse_string_expr(std::vector<void*>& args, void* data){
    lexing::LexToken* str = static_cast<lexing::LexToken*>(args[0]);

    lang::String* expr = new lang::String(str->value);

    delete str;

    return expr;
}

//void* parse_module_stmt_list_1(std::vector<void*>& args, void* data){
//    std::vector<lang::Node*>* module_stmt_list = new std::vector<lang::Node*>;
//
//    lang::Node* module_stmt = static_cast<lang::Node*>(args[0]);
//
//    module_stmt_list->push_back(module_stmt);
//
//    return module_stmt_list;
//}
//
//void* parse_module_stmt_list_2(std::vector<void*>& args, void* data){
//    std::vector<lang::Node*>* module_stmt_list = static_cast<std::vector<lang::Node*>*>(args[0]);
//    lexing::LexToken* newline = static_cast<lexing::LexToken*>(args[1]);
//    lang::Node* module_stmt = static_cast<lang::Node*>(args[2]);
//
//    module_stmt_list->push_back(module_stmt);
//
//    delete newline;
//
//    return module_stmt_list;
//}
//
//void* parse_func_def_module_stmt(std::vector<void*>& args, void* data){
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
