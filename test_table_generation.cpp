#include "lang.h"
#include <cassert>

// Define our rules 
static const std::vector<lang::prod_rule_t> prod_rules = {
    {lang::module_rule, {lang::expr_rule}},  // module : expr
    {lang::expr_rule, {lang::expr_rule, lang::sub_tok, lang::expr_rule}},  // expr : expr SUB expr
    {lang::expr_rule, {lang::expr_rule, lang::add_tok, lang::expr_rule}},  // expr : expr ADD expr
    {lang::expr_rule, {lang::expr_rule, lang::mul_tok, lang::expr_rule}},  // expr : expr MUL expr
    {lang::expr_rule, {lang::expr_rule, lang::div_tok, lang::expr_rule}},  // expr : expr DIV expr
    {lang::expr_rule, {lang::name_tok}},  // expr : NAME 
    {lang::expr_rule, {lang::int_tok}},  // expr : INT
};

// Expected gotos
static const lang::item_set_t clos_expected = {
    {{lang::module_rule, {lang::expr_rule}}, 0},
    {{lang::expr_rule, {lang::expr_rule, lang::sub_tok, lang::expr_rule}}, 0},
    {{lang::expr_rule, {lang::expr_rule, lang::add_tok, lang::expr_rule}}, 0},
    {{lang::expr_rule, {lang::expr_rule, lang::mul_tok, lang::expr_rule}}, 0},
    {{lang::expr_rule, {lang::expr_rule, lang::div_tok, lang::expr_rule}}, 0},
    {{lang::expr_rule, {lang::name_tok}}, 0},
    {{lang::expr_rule, {lang::int_tok}}, 0},
};

static const lang::item_set_t expr_expected = {
    {{lang::module_rule, {lang::expr_rule}}, 1},
    {{lang::expr_rule, {lang::expr_rule, lang::sub_tok, lang::expr_rule}}, 1},
    {{lang::expr_rule, {lang::expr_rule, lang::add_tok, lang::expr_rule}}, 1},
    {{lang::expr_rule, {lang::expr_rule, lang::mul_tok, lang::expr_rule}}, 1},
    {{lang::expr_rule, {lang::expr_rule, lang::div_tok, lang::expr_rule}}, 1},
};

static const lang::item_set_t name_expected = {
    {{lang::expr_rule, {lang::name_tok}}, 1},
};

static const lang::item_set_t int_expected = {
    {{lang::expr_rule, {lang::int_tok}}, 1},
};

void test_closure(){
    const auto& entry = prod_rules.front();
    lang::item_set_t item_set = {{entry, 0}};
    lang::make_closure(item_set, prod_rules);

    // Check the contents 
    // Should really be the same as the existing rules but with positions of 0
    assert(item_set == clos_expected);
    
    // item_set should not change 
    lang::make_closure(item_set, prod_rules);
    assert(item_set == clos_expected);
}

void test_move_pos(){
    // GOTO expressios
    lang::item_set_t expr_item_set = lang::move_pos(clos_expected, lang::expr_rule, prod_rules);
    assert(expr_item_set == expr_expected);
    
    // GOTO name
    lang::item_set_t name_item_set = lang::move_pos(clos_expected, lang::name_tok, prod_rules);
    assert(name_item_set == name_expected);
    
    // GOTO int
    lang::item_set_t int_item_set = lang::move_pos(clos_expected, lang::int_tok, prod_rules);
    assert(int_item_set == int_expected);
}

void test_dfa_creation(){
    auto entry = prod_rules.front();
    lang::item_set_t item_set = {{entry, 0}};
    make_closure(item_set, prod_rules);
    lang::dfa_t dfa = {item_set};
    make_dfa(dfa, prod_rules);

    const auto parse_table = lang::make_parse_table(dfa, prod_rules, prod_rules.front());
    lang::dump_parse_table(parse_table, prod_rules);
}

int main(){
    test_closure();
    test_move_pos();
    test_dfa_creation();

    return 0;
}
