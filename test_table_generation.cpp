#include "lang.h"
#include <cassert>

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
    const auto& entry = lang::LANG_RULES.front();
    lang::item_set_t item_set = {{entry, 0}};
    lang::init_closure(item_set, lang::LANG_RULES);

    // Check the contents 
    // Should really be the same as the existing rules but with positions of 0
    assert(item_set == clos_expected);
    
    // item_set should not change 
    lang::init_closure(item_set, lang::LANG_RULES);
    assert(item_set == clos_expected);
}

void test_move_pos(){
    // GOTO expressios
    lang::item_set_t expr_item_set = lang::move_pos(clos_expected, lang::expr_rule, lang::LANG_RULES);
    assert(expr_item_set == expr_expected);
    
    // GOTO name
    lang::item_set_t name_item_set = lang::move_pos(clos_expected, lang::name_tok, lang::LANG_RULES);
    assert(name_item_set == name_expected);
    
    // GOTO int
    lang::item_set_t int_item_set = lang::move_pos(clos_expected, lang::int_tok, lang::LANG_RULES);
    assert(int_item_set == int_expected);
}

void test_parser_creation(){
    lang::Parser parser(lang::LANG_RULES);
}

int main(){
    test_closure();
    test_move_pos();
    test_parser_creation();

    return 0;
}
