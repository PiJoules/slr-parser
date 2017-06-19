#include "lang.h"

const std::vector<lang::prod_rule_t> lang::LANG_RULES = {
    {lang::module_rule, {lang::expr_rule}},  // module : expr
    {lang::expr_rule, {lang::expr_rule, lang::sub_tok, lang::expr_rule}},  // expr : expr SUB expr
    {lang::expr_rule, {lang::expr_rule, lang::add_tok, lang::expr_rule}},  // expr : expr ADD expr
    {lang::expr_rule, {lang::expr_rule, lang::mul_tok, lang::expr_rule}},  // expr : expr MUL expr
    {lang::expr_rule, {lang::expr_rule, lang::div_tok, lang::expr_rule}},  // expr : expr DIV expr
    {lang::expr_rule, {lang::name_tok}},  // expr : NAME 
    {lang::expr_rule, {lang::int_tok}},  // expr : INT
};
