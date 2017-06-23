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
    lang::make_pr("module", {"module_stmt_list"}, nullptr),
    //{"module_stmt_list", {"module_stmt"}},
    //{"module_stmt_list", {"module_stmt_list", "module_stmt"}},
    //{"module_stmt", {"func_def"}},
    //{"module_stmt", {"NEWLINE"}},

    // Functions 
    //{"func_def", {"DEF", "NAME", "LPAR", "RPAR", "COLON", "func_suite"}},
    //{"func_suite", {"NEWLINE", lang::tokens::INDENT, "func_stmts", lang::tokens::DEDENT}},
    //{"func_stmts", {"func_stmt"}},
    //{"func_stmts", {"func_stmts", "func_stmt"}},
    //{"func_stmt", {"simple_func_stmt", "NEWLINE"}},
    ////{"func_stmt", {"compound_func_stmt", lang::tokens::NEWLINE}},
    //{"simple_func_stmt", {"expr_stmt"}},

    //// Simple statements - one line 
    //{"expr_stmt", {"expr"}},

    //// Binary Expressions
    //{"expr", {"expr", "SUB", "expr"}},
    //{"expr", {"expr", "ADD", "expr"}},
    //{"expr", {"expr", "MUL", "expr"}},
    //{"expr", {"expr", "DIV", "expr"}},

    //// Atoms
    //{"expr", {"NAME"}},
    //{"expr", {"INT"}},
};

/**************** Associativity ***************/ 

const lang::precedence_t lang::LANG_PRECEDENCE = {
    {lang::RIGHT_ASSOC, {"ADD", "SUB"}},
    {lang::LEFT_ASSOC, {"MUL", "DIV"}},
};
