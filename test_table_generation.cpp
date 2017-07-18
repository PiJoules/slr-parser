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

static const std::vector<parsing::ParseRule> test_rules = {
    {"module", {"expr"}, nullptr},
    {"expr", {"expr", "SUB", "expr"}, nullptr},
    {"expr", {"expr", "ADD", "expr"}, nullptr},
    {"expr", {"expr", "MUL", "expr"}, nullptr},
    {"expr", {"expr", "DIV", "expr"}, nullptr},
    {"expr", {"NAME"}, nullptr},
    {"expr", {"INT"}, nullptr},
};

static const parsing::PrecedenceList test_precedence = {
    {parsing::RIGHT_ASSOC, {"ADD", "SUB"}},
    {parsing::LEFT_ASSOC, {"MUL", "DIV"}},
};

static const parsing::LRItemSet clos_expected = {
    {{"module", {"expr"}, nullptr}, 0},
    {{"expr", {"expr", "SUB", "expr"}, nullptr}, 0},
    {{"expr", {"expr", "ADD", "expr"}, nullptr}, 0},
    {{"expr", {"expr", "MUL", "expr"}, nullptr}, 0},
    {{"expr", {"expr", "DIV", "expr"}, nullptr}, 0},
    {{"expr", {"NAME"}, nullptr}, 0},
    {{"expr", {"INT"}, nullptr}, 0},
};

static const parsing::LRItemSet expr_expected = {
    {{"module", {"expr"}, nullptr}, 1},
    {{"expr", {"expr", "SUB", "expr"}, nullptr}, 1},
    {{"expr", {"expr", "ADD", "expr"}, nullptr}, 1},
    {{"expr", {"expr", "MUL", "expr"}, nullptr}, 1},
    {{"expr", {"expr", "DIV", "expr"}, nullptr}, 1},
};

static const parsing::LRItemSet name_expected = {
    {{"expr", {"NAME"}, nullptr}, 1},
};

static const parsing::LRItemSet int_expected = {
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

    const std::vector<parsing::ParseRule> rules = {
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
    parsing::Grammar grammar(lexer, rules);

    // firsts 
    //std::unordered_set<std::string> expected = {"a", "o", "z", "c"};
    //assert(grammar.firsts("S") == expected);
    //expected = {"a", "o", "z"};
    //assert(grammar.firsts("B") == expected);
    //expected = {"z"};
    //assert(grammar.firsts("D") == expected);
    //expected = {"o", "c"};
    //assert(grammar.firsts("C") == expected);

    //// follows 
    //expected = {lexing::tokens::END};
    //assert(grammar.follows("S") == expected);
    //expected = {"b"};
    //assert(grammar.follows("B") == expected);
    //expected = {"d"};
    //assert(grammar.follows("C") == expected);
    //expected = {"a"};
    //assert(grammar.follows("D") == expected);

    //// empty stacks 
    //assert(grammar.firsts_stack().empty());
    //assert(grammar.follows_stack().empty());
}

void test_rules2(){
    const lexing::TokensMap tokens = {
        {"LPAR", {R"(\()", nullptr}},
        {"RPAR", {R"(\))", nullptr}},
        {"n", {"n", nullptr}},
        {"PLUS", {R"(\+)", nullptr}},
    };

    const std::vector<parsing::ParseRule> rules = {
        {"S", {"E"}, nullptr},
        {"E", {"T"}, nullptr},
        {"E", {"LPAR", "E", "RPAR"}, nullptr},
        {"T", {"n"}, nullptr},
        {"T", {"PLUS", "T"}, nullptr},
        {"T", {"T", "PLUS", "T"}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    parsing::Grammar grammar(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"n", "PLUS", "LPAR"};
    assert(grammar.firsts("S") == expected);
    assert(grammar.firsts("E") == expected);
    expected = {"n", "PLUS"};
    assert(grammar.firsts("T") == expected);

    // follows 
    expected = {lexing::tokens::END};
    assert(grammar.follows("S") == expected);
    expected = {"RPAR", lexing::tokens::END};
    assert(grammar.follows("E") == expected);
    expected = {"PLUS", "RPAR", lexing::tokens::END};
    assert(grammar.follows("T") == expected);

    // empty stacks 
    assert(grammar.firsts_stack().empty());
    assert(grammar.follows_stack().empty());
}

void test_rules3(){
    const lexing::TokensMap tokens = {
        {"LPAR", {R"(\()", nullptr}},
        {"RPAR", {R"(\))", nullptr}},
        {"PLUS", {R"(\+)", nullptr}},
        {"MULT", {R"(\*)", nullptr}},
        {"ID", {"id", nullptr}},
    };

    const std::vector<parsing::ParseRule> rules = {
        {"E", {"E", "PLUS", "T"}, nullptr},
        {"E", {"T"}, nullptr},
        {"T", {"T", "MULT", "F"}, nullptr},
        {"T", {"F"}, nullptr},
        {"F", {"LPAR", "E", "RPAR"}, nullptr},
        {"F", {"ID"}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    parsing::Grammar grammar(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"ID", "LPAR"};
    assert(grammar.firsts("E") == expected);
    assert(grammar.firsts("T") == expected);
    assert(grammar.firsts("F") == expected);

    // follows 
    expected = {lexing::tokens::END, "PLUS", "RPAR"};
    assert(grammar.follows("E") == expected);
    expected = {lexing::tokens::END, "PLUS", "RPAR", "MULT"};
    assert(grammar.follows("T") == expected);
    assert(grammar.follows("F") == expected);

    // empty stacks 
    assert(grammar.firsts_stack().empty());
    assert(grammar.follows_stack().empty());
}

void test_rules4(){
    lang::LangLexer lexer(test_tokens);
    parsing::Grammar grammar(lexer, test_rules);

    // firsts
    std::unordered_set<std::string> expected = {"NAME", "INT"};
    assert(grammar.firsts("expr") == expected);
    assert(grammar.firsts("module") == expected);

    // follows 
    expected = {lexing::tokens::END, "ADD", "SUB", "MUL", "DIV"};
    assert(grammar.follows("expr") == expected);
    expected = {lexing::tokens::END};
    assert(grammar.follows("module") == expected);

    // empty stacks 
    assert(grammar.firsts_stack().empty());
    assert(grammar.follows_stack().empty());
}

void test_rules5(){
    const lexing::TokensMap tokens = {
        {"a", {"a", nullptr}},
    };

    const std::vector<parsing::ParseRule> rules = {
        {"S", {"X"}, nullptr},
        {"X", {"a"}, nullptr},
        {"X", {parsing::nonterminals::EPSILON}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    parsing::Grammar grammar(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"a", parsing::nonterminals::EPSILON};
    assert(grammar.firsts("S") == expected);
    assert(grammar.firsts("X") == expected);

    // follows 
    expected = {lexing::tokens::END};
    assert(grammar.follows("S") == expected);
    assert(grammar.follows("X") == expected);

    // empty stacks 
    assert(grammar.firsts_stack().empty());
    assert(grammar.follows_stack().empty());
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
        {lang::tokens::NEWLINE, {R"(\n+)", nullptr}},
        {"TOKEN", {"TOKEN", nullptr}},
    };

    const std::vector<parsing::ParseRule> rules = {
        // Entry point
        {"module", {"module_stmt_list"}, nullptr},
        {"module_stmt_list", {"module_stmt"}, nullptr},
        {"module_stmt_list", {"module_stmt_list", "module_stmt"}, nullptr},
        {"module_stmt", {"func_def"}, nullptr},
        {"module_stmt", {lang::tokens::NEWLINE}, nullptr},

        // Functions 
        {"func_def", {"DEF", "NAME", "LPAR", "RPAR", "COLON", "func_suite"}, nullptr},
        {"func_suite", {lang::tokens::NEWLINE, lang::tokens::INDENT, "func_stmts", lang::tokens::DEDENT}, nullptr},
        {"func_stmts", {"func_stmt"}, nullptr},
        {"func_stmts", {"func_stmts", "func_stmt"}, nullptr},
        {"func_stmt", {"simple_func_stmt", lang::tokens::NEWLINE}, nullptr},
        {"simple_func_stmt", {"TOKEN"}, nullptr},
    };

    lang::LangLexer lexer(tokens);
    parsing::Grammar grammar(lexer, rules);

    // firsts 
    std::unordered_set<std::string> expected = {"DEF", lang::tokens::NEWLINE};
    assert(grammar.firsts("module_stmt") == expected);
    assert(grammar.firsts("module_stmt_list") == expected);
    assert(grammar.firsts("module") == expected);

    // follows 
    expected = {lexing::tokens::END};
    assert(grammar.follows("module") == expected);
    expected = {lexing::tokens::END, "DEF", lang::tokens::NEWLINE};
    assert(grammar.follows("module_stmt") == expected);
    assert(grammar.follows("module_stmt_list") == expected);

    // empty stacks 
    assert(grammar.firsts_stack().empty());
    assert(grammar.follows_stack().empty());
}

void test_closure(){
    const auto& entry = test_rules.front();
    parsing::LRItemSet item_set = {{entry, 0}};
    parsing::init_closure(item_set, test_rules);

    // Check the contents 
    // Should really be the same as the existing rules but with positions of 0
    assert(item_set == clos_expected);
    
    // item_set should not change 
    parsing::init_closure(item_set, test_rules);
    assert(item_set == clos_expected);
}

void test_move_pos(){
    // GOTO expressios
    parsing::LRItemSet expr_item_set = parsing::move_pos(clos_expected, "expr", test_rules);
    assert(expr_item_set == expr_expected);
    
    // GOTO name
    parsing::LRItemSet name_item_set = parsing::move_pos(clos_expected, "NAME", test_rules);
    assert(name_item_set == name_expected);
    
    // GOTO int
    parsing::LRItemSet int_item_set = parsing::move_pos(clos_expected, "INT", test_rules);
    assert(int_item_set == int_expected);
}

void test_parse_precedence(){
    lang::LangLexer lexer(test_tokens);

    // Should have conflicts 
    parsing::Grammar grammar(lexer, test_rules);
    assert(!grammar.conflicts().empty());

    // Should not have conflicts 
    parsing::Grammar grammar2(lexer, test_rules, test_precedence);
    assert(grammar2.conflicts().empty());
}

int main(){
    test_closure();
    test_move_pos();
    test_parse_precedence();

    test_rules1();
    test_rules2();
    test_rules3();
    test_rules4();
    test_rules5();
    test_rules6();

    return 0;
}
