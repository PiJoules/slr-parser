#include "lang.h"
#include <cassert>

static const lexing::TokensMap test_tokens = {
    // Values
    {"INT", {R"(\d+)", nullptr}},
    {"NAME", {R"([a-zA-Z_][a-zA-Z0-9_]*)", nullptr}},

    // Binary operators
    {"ADD", {R"(\+)", nullptr}},
    {"SUB", {R"(-)", nullptr}},
    {"MUL", {R"(\*)", nullptr}},
    {"DIV", {R"(\\)", nullptr}},
};

static const std::vector<lang::ParseRule> test_rules = {
    {"module", {"expr"}, nullptr},
    {"expr", {"expr", "SUB", "expr"}, nullptr},
    {"expr", {"expr", "ADD", "expr"}, nullptr},
    {"expr", {"expr", "MUL", "expr"}, nullptr},
    {"expr", {"expr", "DIV", "expr"}, nullptr},
    {"expr", {"NAME"}, nullptr},
    {"expr", {"INT"}, nullptr},
};

static const lang::precedence_t test_precedence = {
    {lang::RIGHT_ASSOC, {"ADD", "SUB"}},
    {lang::LEFT_ASSOC, {"MUL", "DIV"}},
};

static const lang::item_set_t clos_expected = {
    {{"module", {"expr"}, nullptr}, 0},
    {{"expr", {"expr", "SUB", "expr"}, nullptr}, 0},
    {{"expr", {"expr", "ADD", "expr"}, nullptr}, 0},
    {{"expr", {"expr", "MUL", "expr"}, nullptr}, 0},
    {{"expr", {"expr", "DIV", "expr"}, nullptr}, 0},
    {{"expr", {"NAME"}, nullptr}, 0},
    {{"expr", {"INT"}, nullptr}, 0},
};

static const lang::item_set_t expr_expected = {
    {{"module", {"expr"}, nullptr}, 1},
    {{"expr", {"expr", "SUB", "expr"}, nullptr}, 1},
    {{"expr", {"expr", "ADD", "expr"}, nullptr}, 1},
    {{"expr", {"expr", "MUL", "expr"}, nullptr}, 1},
    {{"expr", {"expr", "DIV", "expr"}, nullptr}, 1},
};

static const lang::item_set_t name_expected = {
    {{"expr", {"NAME"}, nullptr}, 1},
};

static const lang::item_set_t int_expected = {
    {{"expr", {"INT"}, nullptr}, 1},
};

void test_rules1(){
    const lexing::TokensMap tokens = {
        {"a", {"a", nullptr}},
        {"b", {"b", nullptr}},
        {"c", {"c", nullptr}},
        {"d", {"d", nullptr}},
        {"o", {"o", nullptr}},
        {"z", {"z", nullptr}},
    };

    const std::vector<lang::ParseRule> rules = {
        {"S", {"B", "b"}, nullptr},
        {"S", {"C", "d"}, nullptr},
        {"B", {"a", "B"}, nullptr},
        {"B", {"o"}, nullptr},
        {"B", {"D", "a"}, nullptr},
        {"D", {"z"}, nullptr},
        {"C", {"c", "C"}, nullptr},
        {"C", {"o"}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    lang::Parser parser(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"a", "o", "z", "c"};
    assert(parser.firsts("S") == expected);
    expected = {"a", "o", "z"};
    assert(parser.firsts("B") == expected);
    expected = {"z"};
    assert(parser.firsts("D") == expected);
    expected = {"o", "c"};
    assert(parser.firsts("C") == expected);

    // follows 
    expected = {lexing::tokens::END};
    assert(parser.follows("S") == expected);
    expected = {"b"};
    assert(parser.follows("B") == expected);
    expected = {"d"};
    assert(parser.follows("C") == expected);
    expected = {"a"};
    assert(parser.follows("D") == expected);

    // empty stacks 
    assert(parser.firsts_stack().empty());
    assert(parser.follows_stack().empty());
}

void test_rules2(){
    const lexing::TokensMap tokens = {
        {"LPAR", {R"(\()", nullptr}},
        {"RPAR", {R"(\))", nullptr}},
        {"n", {"n", nullptr}},
        {"PLUS", {R"(\+)", nullptr}},
    };

    const std::vector<lang::ParseRule> rules = {
        {"S", {"E"}, nullptr},
        {"E", {"T"}, nullptr},
        {"E", {"LPAR", "E", "RPAR"}, nullptr},
        {"T", {"n"}, nullptr},
        {"T", {"PLUS", "T"}, nullptr},
        {"T", {"T", "PLUS", "T"}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    lang::Parser parser(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"n", "PLUS", "LPAR"};
    assert(parser.firsts("S") == expected);
    assert(parser.firsts("E") == expected);
    expected = {"n", "PLUS"};
    assert(parser.firsts("T") == expected);

    // follows 
    expected = {lexing::tokens::END};
    assert(parser.follows("S") == expected);
    expected = {"RPAR", lexing::tokens::END};
    assert(parser.follows("E") == expected);
    expected = {"PLUS", "RPAR", lexing::tokens::END};
    assert(parser.follows("T") == expected);

    // empty stacks 
    assert(parser.firsts_stack().empty());
    assert(parser.follows_stack().empty());
}

void test_rules3(){
    const lexing::TokensMap tokens = {
        {"LPAR", {R"(\()", nullptr}},
        {"RPAR", {R"(\))", nullptr}},
        {"PLUS", {R"(\+)", nullptr}},
        {"MULT", {R"(\*)", nullptr}},
        {"ID", {"id", nullptr}},
    };

    const std::vector<lang::ParseRule> rules = {
        {"E", {"E", "PLUS", "T"}, nullptr},
        {"E", {"T"}, nullptr},
        {"T", {"T", "MULT", "F"}, nullptr},
        {"T", {"F"}, nullptr},
        {"F", {"LPAR", "E", "RPAR"}, nullptr},
        {"F", {"ID"}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    lang::Parser parser(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"ID", "LPAR"};
    assert(parser.firsts("E") == expected);
    assert(parser.firsts("T") == expected);
    assert(parser.firsts("F") == expected);

    // follows 
    expected = {lexing::tokens::END, "PLUS", "RPAR"};
    assert(parser.follows("E") == expected);
    expected = {lexing::tokens::END, "PLUS", "RPAR", "MULT"};
    assert(parser.follows("T") == expected);
    assert(parser.follows("F") == expected);

    // empty stacks 
    assert(parser.firsts_stack().empty());
    assert(parser.follows_stack().empty());
}

void test_rules4(){
    lang::LangLexer lexer(test_tokens);
    lang::Parser parser(lexer, test_rules);

    // firsts
    std::unordered_set<std::string> expected = {"NAME", "INT"};
    assert(parser.firsts("expr") == expected);
    assert(parser.firsts("module") == expected);

    // follows 
    expected = {lexing::tokens::END, "ADD", "SUB", "MUL", "DIV"};
    assert(parser.follows("expr") == expected);
    expected = {lexing::tokens::END};
    assert(parser.follows("module") == expected);

    // empty stacks 
    assert(parser.firsts_stack().empty());
    assert(parser.follows_stack().empty());
}

void test_rules5(){
    const lexing::TokensMap tokens = {
        {"a", {"a", nullptr}},
    };

    const std::vector<lang::ParseRule> rules = {
        {"S", {"X"}, nullptr},
        {"X", {"a"}, nullptr},
        {"X", {parsing::nonterminals::EPSILON}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    lang::Parser parser(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"a", parsing::nonterminals::EPSILON};
    assert(parser.firsts("S") == expected);
    assert(parser.firsts("X") == expected);

    // follows 
    expected = {lexing::tokens::END};
    assert(parser.follows("S") == expected);
    assert(parser.follows("X") == expected);

    // empty stacks 
    assert(parser.firsts_stack().empty());
    assert(parser.follows_stack().empty());
}

static std::unordered_map<std::string, std::string> RESERVED_NAMES = {
    {"def", "DEF"},
    {"TOKEN", "TOKEN"},
};

static void reserved(lexing::LexToken& tok, void* data){
    if (RESERVED_NAMES.find(tok.value) != RESERVED_NAMES.end()){
        tok.symbol = RESERVED_NAMES[tok.value];
    }
}

void test_rules6(){

    const lexing::TokensMap tokens = {
        // Values
        {"INT", {R"(\d+)", nullptr}},
        {"NAME", {R"([a-zA-Z_][a-zA-Z0-9_]*)", reserved}},

        // Binary operators
        {"ADD", {R"(\+)", nullptr}},
        {"SUB", {R"(-)", nullptr}},
        {"MUL", {R"(\*)", nullptr}},
        {"DIV", {R"(\\)", nullptr}},

        // Containers 
        {"LPAR", {R"(\()", nullptr}},
        {"RPAR", {R"(\))", nullptr}},

        // Misc 
        {"DEF", {R"(def)", nullptr}},
        {"NEWLINE", {R"(\n+)", nullptr}},
        {"TOKEN", {"TOKEN", nullptr}},
    };

    const std::vector<lang::ParseRule> rules = {
        // Entry point
        {"module", {"module_stmt_list"}, nullptr},
        {"module_stmt_list", {"module_stmt"}, nullptr},
        {"module_stmt_list", {"module_stmt_list", "module_stmt"}, nullptr},
        {"module_stmt", {"func_def"}, nullptr},
        {"module_stmt", {"NEWLINE"}, nullptr},

        // Functions 
        {"func_def", {"DEF", "NAME", "LPAR", "RPAR", "COLON", "func_suite"}, nullptr},
        {"func_suite", {"NEWLINE", lang::tokens::INDENT, "func_stmts", lang::tokens::DEDENT}, nullptr},
        {"func_stmts", {"func_stmt"}, nullptr},
        {"func_stmts", {"func_stmts", "func_stmt"}, nullptr},
        {"func_stmt", {"simple_func_stmt", "NEWLINE"}, nullptr},
        {"simple_func_stmt", {"TOKEN"}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    lang::Parser parser(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"DEF", "NEWLINE"};
    assert(parser.firsts("module_stmt") == expected);
    assert(parser.firsts("module_stmt_list") == expected);
    assert(parser.firsts("module") == expected);

    // follows 
    expected = {lexing::tokens::END};
    assert(parser.follows("module") == expected);
    expected = {lexing::tokens::END, "DEF", "NEWLINE"};
    assert(parser.follows("module_stmt") == expected);
    assert(parser.follows("module_stmt_list") == expected);

    // empty stacks 
    assert(parser.firsts_stack().empty());
    assert(parser.follows_stack().empty());
}

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
    lang::LangLexer lexer(test_tokens);

    // Should have conflicts 
    lang::Parser parser(lexer, test_rules);
    assert(!parser.conflicts().empty());

    // Should not have conflicts 
    lang::Parser parser2(lexer, test_rules, test_precedence);
    assert(parser2.conflicts().empty());
}

int main(){
    test_rules1();
    test_rules2();
    test_rules3();
    test_rules4();
    test_rules5();
    test_rules6();

    test_closure();
    test_move_pos();
    test_parse_precedence();

    return 0;
}
