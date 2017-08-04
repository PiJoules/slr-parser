#include "lang.h"

/****************** Lexer tokens *****************/

std::unordered_map<std::string, std::string> RESERVED_NAMES = {
    {"def", "DEF"},
    {"return", "RETURN"},
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
    {"RETURN", {"return", nullptr}},
    {lang::tokens::NEWLINE, {R"(\n+)", nullptr}},
    {"COLON", {R"(\:)", nullptr}},
    {"COMMA", {R"(\,)", nullptr}},
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

// func_def : DEF NAME LPAR RPAR COLON func_suite
void* parse_func_def(std::vector<void*>& args, void* data){
    lexing::LexToken* def = static_cast<lexing::LexToken*>(args[0]);
    lexing::LexToken* name = static_cast<lexing::LexToken*>(args[1]);
    lexing::LexToken* lpar = static_cast<lexing::LexToken*>(args[2]);
    lexing::LexToken* rpar = static_cast<lexing::LexToken*>(args[3]);
    lexing::LexToken* colon = static_cast<lexing::LexToken*>(args[4]);
    std::vector<lang::FuncStmt*>* func_suite = static_cast<std::vector<lang::FuncStmt*>*>(args[5]);
    
    lang::FuncDef* func_def = new lang::FuncDef(name->value, *func_suite);

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
void* parse_func_stmt(std::vector<void*>& args, void* data){
    lang::SimpleFuncStmt* simple_func_stmt = static_cast<lang::SimpleFuncStmt*>(args[0]);
    lexing::LexToken* newline = static_cast<lexing::LexToken*>(args[1]);

    delete newline;

    return simple_func_stmt;
}

// simple_func_stmt : expr_stmt
//                  | return_stmt
void* parse_simple_func_stmt(std::vector<void*>& args, void* data){
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

// expr : expr LPAR call_args RPAR
void* parse_func_call(std::vector<void*>& args, void* data){
    lang::Expr* expr = static_cast<lang::Expr*>(args[0]);
    lexing::LexToken* lpar = static_cast<lexing::LexToken*>(args[1]);
    std::vector<lang::Expr*>* call_args = static_cast<std::vector<lang::Expr*>*>(args[2]);
    lexing::LexToken* rpar = static_cast<lexing::LexToken*>(args[3]);

    delete lpar;
    delete rpar;

    lang::Call* call = new lang::Call(expr, *call_args);

    delete call_args;

    return call;
}

// call_args : call_arg  
void* parse_call_one_arg(std::vector<void*>& args, void* data){
    lang::Expr* call_arg = static_cast<lang::Expr*>(args[0]);

    std::vector<lang::Expr*>* call_args = new std::vector<lang::Expr*>;
    call_args->push_back(call_arg);

    return call_args;
}

// call_args : call_args COMMA call_arg
void* parse_call_args(std::vector<void*>& args, void* data){
    std::vector<lang::Expr*>* call_args = static_cast<std::vector<lang::Expr*>*>(args[0]);
    lexing::LexToken* comma = static_cast<lexing::LexToken*>(args[1]);
    lang::Expr* call_arg = static_cast<lang::Expr*>(args[2]);

    delete comma;

    call_args->push_back(call_arg);

    return call_args;
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

const std::vector<parsing::ParseRule> lang::LANG_RULES = {
    // Entry point 
    {"module", {"module_stmt_list"}, parse_module},
    {"module_stmt_list", {"func_def"}, parse_module_stmt_list},
    {"module_stmt_list", {lang::tokens::NEWLINE}, parse_module_stmt_list2},
    {"module_stmt_list", {"module_stmt_list", lang::tokens::NEWLINE}, parse_module_stmt_list3},
    {"module_stmt_list", {"module_stmt_list", "func_def"}, parse_module_stmt_list4},

    // Functions 
    {"func_def", {"DEF", "NAME", "LPAR", "RPAR", "COLON", "func_suite"}, parse_func_def},
    {"func_suite", {lang::tokens::NEWLINE, lang::tokens::INDENT, "func_stmts", lang::tokens::DEDENT}, parse_func_suite},
    {"func_stmts", {"func_stmt"}, parse_func_stmts},
    {"func_stmts", {"func_stmts", "func_stmt"}, parse_func_stmts2},
    {"func_stmt", {"simple_func_stmt", lang::tokens::NEWLINE}, parse_func_stmt},
    {"simple_func_stmt", {"expr_stmt"}, parse_simple_func_stmt},
    {"simple_func_stmt", {"return_stmt"}, parse_simple_func_stmt},

    // Simple statements - one line 
    {"expr_stmt", {"expr"}, parse_expr_stmt},
    {"return_stmt", {"RETURN", "expr"}, parse_return_stmt},

    // Function calls 
    {"expr", {"expr", "LPAR", "RPAR"}, parse_empty_func_call},
    {"expr", {"expr", "LPAR", "call_args", "RPAR"}, parse_func_call},
    {"call_args", {"call_arg"}, parse_call_one_arg},
    {"call_args", {"call_args", "COMMA", "call_arg"}, parse_call_args},

    // Binary Expressions
    {"expr", {"expr", "SUB", "expr"}, parse_bin_sub_expr},
    {"expr", {"expr", "ADD", "expr"}, parse_bin_add_expr},
    {"expr", {"expr", "MUL", "expr"}, parse_bin_mul_expr},
    {"expr", {"expr", "DIV", "expr"}, parse_bin_div_expr},

    // Unary expressions
    {"expr", {"SUB", "expr", "%UMINUS"}, parse_un_sub_expr},

    // Atoms
    {"expr", {"NAME"}, parse_name_expr},
    {"expr", {"INT"}, parse_int_expr},
};

/**************** Associativity ***************/ 

const parsing::PrecedenceList lang::LANG_PRECEDENCE = {
    {parsing::LEFT_ASSOC, {"ADD", "SUB"}},
    {parsing::RIGHT_ASSOC, {"MUL", "DIV"}},
    {parsing::RIGHT_ASSOC, {"UMINUS"}},

    // Function call
    {parsing::LEFT_ASSOC, {"LPAR"}},
};

/**************** Grammar ***************/ 

const parsing::Grammar lang::LANG_GRAMMAR(parsing::keys(lang::LANG_TOKENS),
                                          lang::LANG_RULES,
                                          lang::LANG_PRECEDENCE);
