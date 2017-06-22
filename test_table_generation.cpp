#include "lang.h"
#include <cassert>

static const std::unordered_map<std::string, std::string> test_tokens = {
    // Values
    {"INT", R"(\d+)"},
    {"NAME", R"([a-zA-Z_][a-zA-Z0-9_]*)"},

    // Binary operators
    {"ADD", R"(\+)"},
    {"SUB", R"(-)"},
    {"MUL", R"(\*)"},
    {"DIV", R"(\\)"},

    // Misc
    {lang::tokens::NEWLINE, R"(\n+)"},  // Capture newlines
};

static const std::vector<lang::prod_rule_t> lang_rules = {
    {"module", {"expr"}},
    {"expr", {"expr", "SUB", "expr"}},
    {"expr", {"expr", "ADD", "expr"}},
    {"expr", {"expr", "MUL", "expr"}},
    {"expr", {"expr", "DIV", "expr"}},
    {"expr", {"NAME"}},
    {"expr", {"INT"}},
};

static const lang::item_set_t clos_expected = {
    {{"module", {"expr"}}, 0},
    {{"expr", {"expr", "SUB", "expr"}}, 0},
    {{"expr", {"expr", "ADD", "expr"}}, 0},
    {{"expr", {"expr", "MUL", "expr"}}, 0},
    {{"expr", {"expr", "DIV", "expr"}}, 0},
    {{"expr", {"NAME"}}, 0},
    {{"expr", {"INT"}}, 0},
};

static const lang::item_set_t expr_expected = {
    {{"module", {"expr"}}, 1},
    {{"expr", {"expr", "SUB", "expr"}}, 1},
    {{"expr", {"expr", "ADD", "expr"}}, 1},
    {{"expr", {"expr", "MUL", "expr"}}, 1},
    {{"expr", {"expr", "DIV", "expr"}}, 1},
};

static const lang::item_set_t name_expected = {
    {{"expr", {"NAME"}}, 1},
};

static const lang::item_set_t int_expected = {
    {{"expr", {"INT"}}, 1},
};

void test_closure(){
    const auto& entry = lang_rules.front();
    lang::item_set_t item_set = {{entry, 0}};
    lang::init_closure(item_set, lang_rules);

    // Check the contents 
    // Should really be the same as the existing rules but with positions of 0
    assert(item_set == clos_expected);
    
    // item_set should not change 
    lang::init_closure(item_set, lang_rules);
    assert(item_set == clos_expected);
}

void test_move_pos(){
    // GOTO expressios
    lang::item_set_t expr_item_set = lang::move_pos(clos_expected, "expr", lang_rules);
    assert(expr_item_set == expr_expected);
    
    // GOTO name
    lang::item_set_t name_item_set = lang::move_pos(clos_expected, "NAME", lang_rules);
    assert(name_item_set == name_expected);
    
    // GOTO int
    lang::item_set_t int_item_set = lang::move_pos(clos_expected, "INT", lang_rules);
    assert(int_item_set == int_expected);
}

void test_parser_creation(){
    lang::Lexer lexer(test_tokens);
    lang::Parser parser(lexer, lang_rules);
}

int main(){
    test_closure();
    test_move_pos();
    test_parser_creation();

    return 0;
}
