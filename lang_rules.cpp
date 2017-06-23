#include "lang.h"

/****************** Lexer tokens *****************/

const std::unordered_map<std::string, std::string> lang::LANG_TOKENS = {
    // Values
    {"INT", R"(\d+)"},
    {"NAME", R"([a-zA-Z_][a-zA-Z0-9_]*)"},

    // Binary operators
    {"ADD", R"(\+)"},
    {"SUB", R"(-)"},
    {"MUL", R"(\*)"},
    {"DIV", R"(\\)"},

    // Containers 
    {"LPAR", R"(\()"},
    {"RPAR", R"(\))"},

    // Misc 
    {"DEF", R"(def)"},
    {"NEWLINE", R"(\n+)"},
};

/********** Parser rules ***************/ 

// module : module_stmt_list
//void parse_module(void* result, const std::vector<void*>& args){
//    lang::Module* mod = static_cast<lang::Module*>(result);
//}

const std::vector<lang::prod_rule_t> lang::LANG_RULES = {
    // Entry point
    lang::make_pr("module", {"module_stmt_list"}),
    lang::make_pr("module_stmt_list", {"module_stmt"}),
    lang::make_pr("module_stmt_list", {"module_stmt_list", "module_stmt"}),
    lang::make_pr("module_stmt", {"func_def"}),
    lang::make_pr("module_stmt", {"NEWLINE"}),

    // Functions 
    lang::make_pr("func_def", {"DEF", "NAME", "LPAR", "RPAR", "COLON", "func_suite"}),
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
