#include "lang.h"

/****************** Lexer tokens *****************/

typedef lang::LexToken (*scan_method)(lang::Lexer&);

/**
 * int = \d+
 */
lang::LexToken scan_int(lang::Lexer& lexer){
    std::string num;
    lang::LexToken tok = lexer.spawn_tok(lang::int_tok);
    do {
        num += lexer.getc();
    } while (isdigit(lexer.peekc()));
    tok.value = num;
    return tok;
}

static bool valid_name_char(char c){
    return isalnum(c) || c == UNDERSCORE_C;
}

/**
 * name = [A-Za-z_][A-Za-z0-9_]*
 */
lang::LexToken scan_name(lang::Lexer& lexer){
    std::string name;
    lang::LexToken tok = lexer.spawn_tok(lang::name_tok);
    do {
        name += lexer.getc();
    } while (valid_name_char(lexer.peekc()));
    tok.value = name;
    return tok;
}

lang::LexToken scan_add(lang::Lexer& lexer){
    lang::LexToken tok = lexer.spawn_tok(lang::add_tok);
    tok.value = lexer.getc();
    return tok;
}

const std::unordered_map<std::string, std::string> LANG_TOKENS = {
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
    {lang::module_rule, {lang::expr_rule}},  // module : expr
    {lang::expr_rule, {lang::expr_rule, lang::sub_tok, lang::expr_rule}},  // expr : expr SUB expr
    {lang::expr_rule, {lang::expr_rule, lang::add_tok, lang::expr_rule}},  // expr : expr ADD expr
    {lang::expr_rule, {lang::expr_rule, lang::mul_tok, lang::expr_rule}},  // expr : expr MUL expr
    {lang::expr_rule, {lang::expr_rule, lang::div_tok, lang::expr_rule}},  // expr : expr DIV expr
    {lang::expr_rule, {lang::name_tok}},  // expr : NAME 
    {lang::expr_rule, {lang::int_tok}},  // expr : INT
};
