#include "lang.h"

/****************** Lexer tokens *****************/

std::unordered_map<std::string, std::string> RESERVED_NAMES = {
    {"def", "DEF"},
};

void reserved_name(lexing::LexToken& tok, void* data){
    if (RESERVED_NAMES.find(tok.value) != RESERVED_NAMES.end()){
        tok.symbol = RESERVED_NAMES[tok.value];
    }
}

void comment(lexing::LexToken& tok, void* data){
    tok.symbol = lexing::tokens::COMMENT;
}

const lexing::TokensMap lang::LANG_TOKENS = {
    // Values
    {"INT", {R"(\d+)", nullptr}},
    {"NAME", {R"([a-zA-Z_][a-zA-Z0-9_]*)", reserved_name}},

    // Binary operators
    {"ADD", {R"(\+)", nullptr}},
    {"SUB", {R"(-)", nullptr}},
    {"MUL", {R"(\*)", nullptr}},
    {"DIV", {R"(\\)", nullptr}},

    // Containers 
    {"LPAR", {R"(\()", nullptr}},
    {"RPAR", {R"(\))", nullptr}},

    // Misc 
    {"DEF", {R"(def)", nullptr}},
    {"NEWLINE", {R"(\n+)", nullptr}},
    {"COLON", {R"(\:)", nullptr}},
    {"WS", {R"([ ]+)", comment}},
    {lang::tokens::INDENT, {lang::tokens::INDENT, nullptr}},
    {lang::tokens::DEDENT, {lang::tokens::DEDENT, nullptr}},
};

/********** Parser rules ***************/ 

// module : module_stmt_list
void* parse_module(std::vector<void*>& args, void* data){
    std::vector<lang::ModuleStmt*>* module_stmt_list = static_cast<std::vector<lang::ModuleStmt*>*>(args[0]);

    lang::Module* module = new lang::Module(*module_stmt_list);

    delete module_stmt_list;

    return module;
}

// module_stmt_list : module_stmt 
void* parse_module_stmt_list(std::vector<void*>& args, void* data){
    lang::ModuleStmt* module_stmt = static_cast<lang::ModuleStmt*>(args[0]);
    std::vector<lang::ModuleStmt*>* module_stmt_list = new std::vector<lang::ModuleStmt*>;
    module_stmt_list->push_back(module_stmt);

    return module_stmt_list;
}

// module_stmt_list : module_stmt_list module_stmt
void* parse_module_stmt_list2(std::vector<void*>& args, void* data){
    lang::ModuleStmt* module_stmt = static_cast<lang::ModuleStmt*>(args[1]);
    std::vector<lang::ModuleStmt*>* module_stmt_list = static_cast<std::vector<lang::ModuleStmt*>*>(args[0]);
    module_stmt_list->push_back(module_stmt);

    return module_stmt_list;
}

// module_stmt : func_def
void* parse_module_stmt(std::vector<void*>& args, void* data){
    lang::FuncDef* func_def = static_cast<lang::FuncDef*>(args[0]);
    return func_def;
}

// module_stmt : NEWLINE
void* parse_module_stmt2(std::vector<void*>& args, void* data){
    lang::Newline* newline = static_cast<lang::Newline*>(args[0]);
    return newline;
}

// func_def : DEF NAME LPAR RPAR COLON func_suite
void* parse_func_def(std::vector<void*>& args, void* data){
    parsing::LexTokenWrapper* def = static_cast<parsing::LexTokenWrapper*>(args[0]);
    parsing::LexTokenWrapper* name = static_cast<parsing::LexTokenWrapper*>(args[1]);
    parsing::LexTokenWrapper* lpar = static_cast<parsing::LexTokenWrapper*>(args[2]);
    parsing::LexTokenWrapper* rpar = static_cast<parsing::LexTokenWrapper*>(args[3]);
    parsing::LexTokenWrapper* colon = static_cast<parsing::LexTokenWrapper*>(args[4]);
    std::vector<lang::FuncStmt*>* func_suite = static_cast<std::vector<lang::FuncStmt*>*>(args[5]);
    
    lang::FuncDef* func_def = new lang::FuncDef(name->token().value, *func_suite);

    delete def;
    delete name;
    delete lpar;
    delete rpar;
    delete colon;
    delete func_suite;

    return func_def;
}

// func_suite : NEWLINE INDENT func_stmts DEDENT
void* parse_func_suite(std::vector<void*>& args, void* data){
    parsing::LexTokenWrapper* newline = static_cast<parsing::LexTokenWrapper*>(args[0]);
    parsing::LexTokenWrapper* indent = static_cast<parsing::LexTokenWrapper*>(args[1]);
    std::vector<lang::FuncStmt*>* func_stmts = static_cast<std::vector<lang::FuncStmt*>*>(args[2]);
    parsing::LexTokenWrapper* dedent = static_cast<parsing::LexTokenWrapper*>(args[3]);

    delete newline;
    delete indent;
    delete dedent;

    return func_stmts;
}

// func_stmts : func_stmt 
void* parse_func_stmts(std::vector<void*>& args, void* data){
    lang::FuncStmt* func_stmt = static_cast<lang::FuncStmt*>(args[0]);
    std::vector<lang::FuncStmt*>* func_stmts = new std::vector<lang::FuncStmt*>;
    func_stmts->push_back(func_stmt);

    return func_stmts;
}

// func_stmts : func_stmts func_stmt 
void* parse_func_stmts2(std::vector<void*>& args, void* data){
    lang::FuncStmt* func_stmt = static_cast<lang::FuncStmt*>(args[1]);
    std::vector<lang::FuncStmt*>* func_stmts = static_cast<std::vector<lang::FuncStmt*>*>(args[0]);
    func_stmts->push_back(func_stmt);

    return func_stmts;
}

// func_stmts : func_stmts NEWLINE func_stmt 
void* parse_func_stmts3(std::vector<void*>& args, void* data){
    lang::FuncStmt* func_stmt = static_cast<lang::FuncStmt*>(args[2]);
    parsing::LexTokenWrapper* newline = static_cast<parsing::LexTokenWrapper*>(args[1]);
    std::vector<lang::FuncStmt*>* func_stmts = static_cast<std::vector<lang::FuncStmt*>*>(args[0]);
    func_stmts->push_back(func_stmt);

    delete newline;

    return func_stmts;
}

// func_stmt : simple_func_stmt NEWLINE
void* parse_func_stmt(std::vector<void*>& args, void* data){
    lang::SimpleFuncStmt* simple_func_stmt = static_cast<lang::SimpleFuncStmt*>(args[0]);
    parsing::LexTokenWrapper* newline = static_cast<parsing::LexTokenWrapper*>(args[1]);

    delete newline;

    return simple_func_stmt;
}

// simple_func_stmt : expr_stmt
void* parse_simple_func_stmt(std::vector<void*>& args, void* data){
    return args[0];
}

// expr_stmt : expr 
void* parse_expr_stmt(std::vector<void*>& args, void* data){
    lang::Expr* expr = static_cast<lang::Expr*>(args[0]);
    lang::ExprStmt* expr_stmt = new lang::ExprStmt(expr);
    
    return expr_stmt;
}

// expr : expr SUB expr 
void* parse_bin_sub_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    parsing::LexTokenWrapper* op_tok = static_cast<parsing::LexTokenWrapper*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Sub* op = new lang::Sub;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr ADD expr 
void* parse_bin_add_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    parsing::LexTokenWrapper* op_tok = static_cast<parsing::LexTokenWrapper*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Add* op = new lang::Add;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr MUL expr 
void* parse_bin_mul_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    parsing::LexTokenWrapper* op_tok = static_cast<parsing::LexTokenWrapper*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Mul* op = new lang::Mul;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : expr DIV expr 
void* parse_bin_div_expr(std::vector<void*>& args, void* data){
    lang::Expr* expr1 = static_cast<lang::Expr*>(args[0]);
    parsing::LexTokenWrapper* op_tok = static_cast<parsing::LexTokenWrapper*>(args[1]);
    lang::Expr* expr2 = static_cast<lang::Expr*>(args[2]);

    lang::Div* op = new lang::Div;
    lang::BinExpr* bin_expr = new lang::BinExpr(expr1, op, expr2);

    delete op_tok;

    return bin_expr;
}

// expr : NAME 
void* parse_name_expr(std::vector<void*>& args, void* data){
    parsing::LexTokenWrapper* name = static_cast<parsing::LexTokenWrapper*>(args[0]);
    lang::NameExpr* expr = new lang::NameExpr(name->token().value);

    delete name;

    return expr;
}

// expr : INT
void* parse_int_expr(std::vector<void*>& args, void* data){
    parsing::LexTokenWrapper* int_tok = static_cast<parsing::LexTokenWrapper*>(args[0]);
    lang::Int* expr = new lang::Int(int_tok->token().value);
    
    delete int_tok;

    return expr;
}

const std::vector<parsing::ParseRule> lang::LANG_RULES = {
    // Entry point 
    {"module", {"module_stmt_list"}, parse_module},
    {"module_stmt_list", {"module_stmt"}, parse_module_stmt_list},
    {"module_stmt_list", {"module_stmt_list", "module_stmt"}, parse_module_stmt_list2},
    {"module_stmt", {"func_def"}, parse_module_stmt},
    {"module_stmt", {"NEWLINE"}, parse_module_stmt2},

    // Functions 
    {"func_def", {"DEF", "NAME", "LPAR", "RPAR", "COLON", "func_suite"}, parse_func_def},
    {"func_suite", {"NEWLINE", lang::tokens::INDENT, "func_stmts", lang::tokens::DEDENT}, parse_func_suite},
    {"func_stmts", {"func_stmt"}, parse_func_stmts},
    {"func_stmts", {"func_stmts", "func_stmt"}, parse_func_stmts2},
    {"func_stmt", {"simple_func_stmt", "NEWLINE"}, parse_func_stmt},
    {"simple_func_stmt", {"expr_stmt"}, parse_simple_func_stmt},

    // Simple statements - one line 
    {"expr_stmt", {"expr"}, parse_expr_stmt},

    // Binary Expressions
    {"expr", {"expr", "SUB", "expr"}, parse_bin_sub_expr},
    {"expr", {"expr", "ADD", "expr"}, parse_bin_add_expr},
    {"expr", {"expr", "MUL", "expr"}, parse_bin_mul_expr},
    {"expr", {"expr", "DIV", "expr"}, parse_bin_div_expr},

    // Atoms
    {"expr", {"NAME"}, parse_name_expr},
    {"expr", {"INT"}, parse_int_expr},
};

/**************** Associativity ***************/ 

const parsing::precedence_t lang::LANG_PRECEDENCE = {
    {parsing::LEFT_ASSOC, {"ADD", "SUB"}},
    {parsing::RIGHT_ASSOC, {"MUL", "DIV"}},
};
