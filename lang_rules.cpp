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
};

/********** Parser rules ***************/

const std::vector<lang::prod_rule_t> lang::LANG_RULES = {
    {"module", {"expr"}},
    {"expr", {"expr", "SUB", "expr"}},
    {"expr", {"expr", "ADD", "expr"}},
    {"expr", {"expr", "MUL", "expr"}},
    {"expr", {"expr", "DIV", "expr"}},
    {"expr", {"NAME"}},
    {"expr", {"INT"}},
};
