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
};

static const std::vector<lang::prod_rule_t> test_rules = {
    lang::make_pr("module", {"expr"}),
    lang::make_pr("expr", {"expr", "SUB", "expr"}),
    lang::make_pr("expr", {"expr", "ADD", "expr"}),
    lang::make_pr("expr", {"expr", "MUL", "expr"}),
    lang::make_pr("expr", {"expr", "DIV", "expr"}),
    lang::make_pr("expr", {"NAME"}),
    lang::make_pr("expr", {"INT"}),
};

static const lang::precedence_t test_precedence = {
    {lang::RIGHT_ASSOC, {"ADD", "SUB"}},
    {lang::LEFT_ASSOC, {"MUL", "DIV"}},
};

static const lang::item_set_t clos_expected = {
    {lang::make_pr("module", {"expr"}), 0},
    {lang::make_pr("expr", {"expr", "SUB", "expr"}), 0},
    {lang::make_pr("expr", {"expr", "ADD", "expr"}), 0},
    {lang::make_pr("expr", {"expr", "MUL", "expr"}), 0},
    {lang::make_pr("expr", {"expr", "DIV", "expr"}), 0},
    {lang::make_pr("expr", {"NAME"}), 0},
    {lang::make_pr("expr", {"INT"}), 0},
};

static const lang::item_set_t expr_expected = {
    {lang::make_pr("module", {"expr"}), 1},
    {lang::make_pr("expr", {"expr", "SUB", "expr"}), 1},
    {lang::make_pr("expr", {"expr", "ADD", "expr"}), 1},
    {lang::make_pr("expr", {"expr", "MUL", "expr"}), 1},
    {lang::make_pr("expr", {"expr", "DIV", "expr"}), 1},
};

static const lang::item_set_t name_expected = {
    {lang::make_pr("expr", {"NAME"}), 1},
};

static const lang::item_set_t int_expected = {
    {lang::make_pr("expr", {"INT"}), 1},
};

void test_closure(){
    const auto& entry = test_rules.front();
    lang::item_set_t item_set = {{entry, 0}};
    lang::init_closure(item_set, test_rules);

    // Check the contents 
    // Should really be the same as the existing rules but with positions of 0
    assert(item_set == clos_expected);
    
    // item_set should not change 
    lang::init_closure(item_set, test_rules);
    assert(item_set == clos_expected);
}

void test_move_pos(){
    // GOTO expressios
    lang::item_set_t expr_item_set = lang::move_pos(clos_expected, "expr", test_rules);
    assert(expr_item_set == expr_expected);
    
    // GOTO name
    lang::item_set_t name_item_set = lang::move_pos(clos_expected, "NAME", test_rules);
    assert(name_item_set == name_expected);
    
    // GOTO int
    lang::item_set_t int_item_set = lang::move_pos(clos_expected, "INT", test_rules);
    assert(int_item_set == int_expected);
}

void test_parse_precedence(){
    lang::Lexer lexer(test_tokens);

    // Should have conflicts 
    lang::Parser parser(lexer, test_rules);

    std::unordered_set<std::string> expected = {"INT", "NAME"};
    assert(parser.firsts("expr") == expected);
    assert(parser.firsts("module") == expected);

    //expected = {"ADD", "SUB", "MUL", "DIV"};
    //assert(parser.follow("expr") == expected);
    //assert(parser.follow("module") == expected);

    //if (parser.conflicts().empty()){
    //    parser.dump_grammar();
    //}
    //assert(!parser.conflicts().empty());

    //// Should not have conflicts 
    //lang::Parser parser2(lexer, test_rules, test_precedence);
    //assert(parser2.conflicts().empty());
}

int main(){
    test_closure();
    test_move_pos();
    test_parse_precedence();

    return 0;
}
