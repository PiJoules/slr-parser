#include "lang.h"


// Production rules to be set at compile time
static const std::vector<lang::prod_rule_t> prod_rules = {
    {lang::expr_rule, {lang::name_tok}},
    {lang::expr_rule, {lang::int_tok}},
};

std::size_t lang::SymbolHasher::operator()(const enum Symbol& symbol) const {
    return static_cast<std::size_t>(symbol);
}

bool lang::is_token(const enum lang::Symbol& symbol){
    return static_cast<int>(symbol) < 0;
}

bool lang::is_rule(const enum lang::Symbol& symbol){
    return static_cast<int>(symbol) > 0;
}

/**
 * Borrowed from python3.6's tuple hash
 */
std::size_t lang::ProdRuleHasher::operator()(const prod_rule_t& prod_rule) const {
    auto rule = prod_rule.first;
    auto prod = prod_rule.second;
    SymbolHasher hasher;
    std::size_t rule_hash = hasher(rule);

    std::size_t hash_mult = _HASH_MULTIPLIER;
    std::size_t prod_hash = 0x345678;
    std::size_t len = prod.size();
    for (const auto& r : prod){
        prod_hash = (prod_hash ^ hasher(r)) * hash_mult;
        hash_mult += 82520 + len + len;
    }
    prod_hash += 97531;
    return prod_hash ^ rule_hash;
}

std::size_t lang::ItemHasher::operator()(const lang::lr_item_t& lr_item) const {
    ProdRuleHasher hasher;
    return hasher(lr_item.first) ^ static_cast<std::size_t>(lr_item.second);
}

/**
 * Borrowed from python3.6's frozenset hash
 */ 
static std::size_t _shuffle_bits(std::size_t h){
    return ((h ^ 89869747UL) ^ (h << 16)) * 3644798167UL;
}

std::size_t lang::ItemSetHasher::operator()(const lang::item_set_t& item_set) const {
    std::size_t hash = 0;
    ItemHasher item_hasher;
    for (const auto& lr_item : item_set){
        hash ^= _shuffle_bits(item_hasher(lr_item));  // entry hashes
    }
    hash ^= item_set.size() * 1927868237UL;  // # of active entrues
    hash = hash * 69069U + 907133923UL;
    return hash;
};

/**
 * Initialize a closure from an item set and list of productions.
 */
void lang::make_closure(lang::item_set_t& item_set, const std::vector<lang::prod_rule_t>& prod_rules){
    std::size_t last_size;

    do {
        last_size = item_set.size();
        for (const auto& item : item_set){
            std::size_t pos = item.second;  // dot position
            lang::production_t prod = item.first.second;  // production list
            if (pos < prod.size()){
                enum lang::Symbol next_symbol = prod[pos];
                
                // Find all productions that start with the next_symbol
                // TODO: See if we can optomize this later
                for (const auto prod_rule : prod_rules){
                    if (next_symbol == prod_rule.first){
                        item_set.insert({prod_rule, 0});
                    }
                }
            }
        }
    } while (item_set.size() != last_size); // while the item set is changing
}

/**
 * Move the parser position over by 1.
 */ 
lang::item_set_t lang::move_pos(const lang::item_set_t& item_set, 
                                const enum lang::Symbol& symbol,
                                const std::vector<lang::prod_rule_t>& prod_rules){
    item_set_t moved_item_set;
    for (const auto lr_item : item_set){
        auto prod_rule = lr_item.first;
        const production_t& prod = prod_rule.second;
        std::size_t pos = lr_item.second;

        if (pos < prod.size()){
            if (prod[pos] == symbol){
                moved_item_set.insert({prod_rule, pos + 1});
            }
        }
    }
    make_closure(moved_item_set, prod_rules);
    return item_set_t(moved_item_set);
}

/**
 * Create the canonical collections of the DFA.
 */ 
void lang::make_dfa(lang::dfa_t& dfa, const std::vector<lang::prod_rule_t>& prod_rules){
    std::size_t last_size;
    do {
        last_size = dfa.size();
        for (const auto& item_set : dfa){
            for (const auto& lr_item : item_set){
                auto production = lr_item.first.second;
                std::size_t pos = lr_item.second;
                if (pos < production.size()){
                    auto next_symbol = production[pos];
                    auto moved_item_set = lang::move_pos(item_set, next_symbol, prod_rules);
                    dfa.insert(moved_item_set);
                }
            }
        }
    } while (dfa.size() != last_size);  // while dfa did not change
}

/**
 * Create parse table from dfa.
 */ 
lang::parse_table_t lang::make_parse_table(const lang::dfa_t&, const std::vector<lang::prod_rule_t>&){
    parse_table_t parse_table;

    // 

    return parse_table;
}

void lang::Parser::input(const std::string& code){
    lexer.input(code);
}

/**
 * Returns true if the value of the next token from the lexer matches
 * what we expect.
 */
bool lang::Parser::check_terminal(const std::string& terminal) const {
    return lexer.peek().value == terminal;
}

/**
 * Pop a token from the lexer.
 */
void lang::Parser::accept_terminal(const std::string& terminal) {
    assert(check_terminal(terminal));
    lexer.token();
}


/**
 * module : module_stmt_list
 */
lang::LangNode lang::Parser::parse_module(){
    Module mod(parse_module_stmt_list());
    return mod;
}

/**
 * module_stmt_list : module_stmt 
 *                  | NEWLINE
 *                  | module_stmt module_stmt_list
 *                  | NEWLINE module_stmt_list
 *                  | empty
 */
std::vector<lang::ModuleStmt> lang::Parser::parse_module_stmt_list(){
    std::vector<ModuleStmt> stmt_lst;

    //while (1){
    //    if (check_module_stmt()){
    //        stmt_lst.push_back(parse_module_stmt());
    //    }
    //    else if (check_terminal("\n")){
    //        accept_terminal();
    //    }
    //    else {
    //        std::ostringstream err;
    //        err << "Unknown terminal '" << 
    //        throw std::runtime_error();
    //    }
    //}

    return stmt_lst;
}

/**
 * module_stmt : funcdef
 */ 


/**
 * funcdef : DEF NAME LPAR RPAR COLON func_suite
 */ 

/**
 * suite : NEWLINE INDENT func_stmts DEDENT
 */ 

/**
 * func_stmts : func_stmt func_stmts
 *            | func_stmt
 */ 

/**
 * func_stmt : simple_func_stmt NEWLINE
 *           | compuound_func_stmt
 */ 

/**
 * simple_func_stmt : small_stmt NEWLINE
 */ 

/**
 * small_stmt : expr_stmt
 */ 

/**
 * expr_stmt : expr
 */ 

/**
 * Priority highest to lowest
 * expr : expr ADD expr 
 *      | expr SUB expr 
 *      | NAME 
 *      | int
 */ 
bool lang::Parser::check_expr() const {
    return check_name() || check_int();
}

//lang::Value lang::Parser::parse_expr(){
//    if (check_name()){
//    }
//    else if (check_int()){
//    }
//    else {
//    }
//}

bool lang::Parser::check_name() const {
    return lexer.peek().symbol == name_tok;
}

bool lang::Parser::check_int() const {
    return lexer.peek().symbol == int_tok;
}
