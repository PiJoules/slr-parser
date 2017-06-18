#include "lang.h"
#include <cassert>

void test_closure(){
    // Define our rules 
    const std::vector<lang::prod_rule_t> prod_rules = {
        {lang::module_rule, {lang::expr_rule}},  // module : expr
        {lang::expr_rule, {lang::expr_rule, lang::sub_tok, lang::expr_rule}},  // expr : expr SUB expr
        {lang::expr_rule, {lang::expr_rule, lang::add_tok, lang::expr_rule}},  // expr : expr ADD expr
        {lang::expr_rule, {lang::expr_rule, lang::mul_tok, lang::expr_rule}},  // expr : expr MUL expr
        {lang::expr_rule, {lang::expr_rule, lang::div_tok, lang::expr_rule}},  // expr : expr DIV expr
        {lang::expr_rule, {lang::name_tok}},  // expr : NAME 
        {lang::expr_rule, {lang::int_tok}},  // expr : INT
    };

    const auto& entry = prod_rules.front();
    lang::item_set_t item_set = {{entry, 0}};
    lang::make_closure(item_set, prod_rules);

    // Check the contents 
    // Should really be the same as the existing rules but with positions of 0
    lang::item_set_t expected = {
        {entry, 0},
        {{lang::expr_rule, {lang::expr_rule, lang::sub_tok, lang::expr_rule}}, 0},
        {{lang::expr_rule, {lang::expr_rule, lang::add_tok, lang::expr_rule}}, 0},
        {{lang::expr_rule, {lang::expr_rule, lang::mul_tok, lang::expr_rule}}, 0},
        {{lang::expr_rule, {lang::expr_rule, lang::div_tok, lang::expr_rule}}, 0},
        {{lang::expr_rule, {lang::name_tok}}, 0},
        {{lang::expr_rule, {lang::int_tok}}, 0},
    };
    assert(item_set == expected);
}

int main(){
    test_closure();

    return 0;
}
