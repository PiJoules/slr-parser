#include "lang.h"

/****************** Lexer tokens *****************/

std::unordered_map<std::string, std::string> RESERVED_NAMES = {
    {"def", "DEF"},
};

lang::LexToken reserved_name(lang::Lexer* lexer, lang::LexToken tok){
    assert(lexer);
    if (RESERVED_NAMES.find(tok.value) == RESERVED_NAMES.end()){
        return tok;
    }
    tok.symbol = RESERVED_NAMES[tok.value];
    return tok;
}

const lang::tokens_map_t lang::LANG_TOKENS = {
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
    {lang::tokens::INDENT, {lang::tokens::INDENT, nullptr}},
    {lang::tokens::DEDENT, {lang::tokens::DEDENT, nullptr}},
};

/********** Parser rules ***************/ 

template<typename Target>
struct NodeCast {
    Target* operator()(lang::Node* node) const {
        return reinterpret_cast<Target*>(node);
    }
};

void* parse_module_prime(std::vector<void*>& args){
    return args.front();
}

// module : module_stmt_list
void* parse_module(std::vector<void*>& args){
    std::vector<lang::ModuleStmt*>* module_stmt_list = static_cast<std::vector<lang::ModuleStmt*>*>(args[0]);

    lang::Module* module = new lang::Module(*module_stmt_list);
    delete module_stmt_list;
    return module;
}

// module_stmt_list : module_stmt 
void* parse_module_stmt_list(std::vector<void*>& args){
    lang::ModuleStmt* module_stmt = static_cast<lang::ModuleStmt*>(args[0]);
    std::vector<lang::ModuleStmt*>* module_stmt_list = new std::vector<lang::ModuleStmt*>;
    module_stmt_list->push_back(module_stmt);

    return module_stmt_list;
}

// module_stmt_list : module_stmt_list module_stmt
void* parse_module_stmt_list2(std::vector<void*>& args){
    lang::ModuleStmt* module_stmt = static_cast<lang::ModuleStmt*>(args[1]);
    std::vector<lang::ModuleStmt*>* module_stmt_list = static_cast<std::vector<lang::ModuleStmt*>*>(args[0]);
    module_stmt_list->push_back(module_stmt);

    return module_stmt_list;
}

// module_stmt : func_def
void* parse_module_stmt(std::vector<void*>& args){
    lang::FuncDef* func_def = static_cast<lang::FuncDef*>(args[0]);
    return func_def;
}

// module_stmt : NEWLINE
void* parse_module_stmt2(std::vector<void*>& args){
    lang::Newline* newline = static_cast<lang::Newline*>(args[0]);
    return newline;
}

// func_def : DEF NAME LPAR RPAR COLON func_suite
void* parse_func_def(std::vector<void*>& args){
    lang::LexTokenWrapper* def = static_cast<lang::LexTokenWrapper*>(args[0]);
    lang::LexTokenWrapper* name = static_cast<lang::LexTokenWrapper*>(args[1]);
    lang::LexTokenWrapper* lpar = static_cast<lang::LexTokenWrapper*>(args[2]);
    lang::LexTokenWrapper* rpar = static_cast<lang::LexTokenWrapper*>(args[3]);
    lang::LexTokenWrapper* colon = static_cast<lang::LexTokenWrapper*>(args[4]);
    lang::FuncSuite* func_suite = static_cast<lang::FuncSuite*>(args[5]);
    
    lang::FuncDef* func_def = new lang::FuncDef(name->token().value, func_suite);

    // Delete unused ones
    delete def;
    delete lpar;
    delete rpar;
    delete colon;

    return func_def;
}

const std::vector<lang::prod_rule_t> lang::LANG_RULES = {
    // Entry point 
    lang::make_pr("module", {"module_stmt_list"}, parse_module),
    lang::make_pr("module_stmt_list", {"module_stmt"}, parse_module_stmt_list),
    lang::make_pr("module_stmt_list", {"module_stmt_list", "module_stmt"}, parse_module_stmt_list2),
    lang::make_pr("module_stmt", {"func_def"}, parse_module_stmt),
    lang::make_pr("module_stmt", {"NEWLINE"}, parse_module_stmt2),

    // Functions 
    lang::make_pr("func_def", {"DEF", "NAME", "LPAR", "RPAR", "COLON", "func_suite"}, parse_func_def),
    lang::make_pr("func_suite", {"NEWLINE", lang::tokens::INDENT, "func_stmts", lang::tokens::DEDENT}),
    lang::make_pr("func_stmts", {"func_stmt"}),
    lang::make_pr("func_stmts", {"func_stmts", "func_stmt"}),
    lang::make_pr("func_stmt", {"simple_func_stmt", "NEWLINE"}),
    //lang::make_pr({"func_stmt", {"compound_func_stmt", lang::tokens::NEWLINE}),
    lang::make_pr("simple_func_stmt", {"expr_stmt"}),

    // Simple statements - one line 
    lang::make_pr("expr_stmt", {"expr"}),

    // Binary Expressions
    lang::make_pr("expr", {"expr", "SUB", "expr"}),
    lang::make_pr("expr", {"expr", "ADD", "expr"}),
    lang::make_pr("expr", {"expr", "MUL", "expr"}),
    lang::make_pr("expr", {"expr", "DIV", "expr"}),

    // Atoms
    lang::make_pr("expr", {"NAME"}),
    lang::make_pr("expr", {"INT"}),
};

/**************** Associativity ***************/ 

const lang::precedence_t lang::LANG_PRECEDENCE = {
    {lang::LEFT_ASSOC, {"ADD", "SUB"}},
    {lang::RIGHT_ASSOC, {"MUL", "DIV"}},
};
