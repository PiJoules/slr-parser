#include "lang.h"


// Production rules to be set at compile time
static const std::vector<lang::prod_rule_t> prod_rules = {
    {lang::expr_rule, {lang::name_tok}},
    {lang::expr_rule, {lang::int_tok}},
};

bool lang::is_token(const enum lang::Symbol& symbol){
    return static_cast<int>(symbol) < 0;
}

bool lang::is_rule(const enum lang::Symbol& symbol){
    return static_cast<int>(symbol) > 0;
}

/**
 * Borrowed from python's tuple hash
 */
std::size_t lang::ProdRuleHasher::operator()(const prod_rule_t& prod_rule) const {
    auto rule = prod_rule.first;
    auto prod = prod_rule.second;
    std::size_t rule_hash = static_cast<std::size_t>(rule);

    std::size_t hash_mult = _HASH_MULTIPLIER;
    std::size_t prod_hash = 0x345678;
    std::size_t len = prod.size();
    for (const auto& r : prod){
        prod_hash = (prod_hash ^ static_cast<std::size_t>(r)) * hash_mult;
        hash_mult += 82520 + len + len;
    }
    prod_hash += 97531;
    return prod_hash ^ rule_hash;
}

std::size_t lang::ItemHasher::operator()(const lr_item_t& lr_item) const {
    ProdRuleHasher hasher;
    return hasher(lr_item.first) ^ static_cast<std::size_t>(lr_item.second);
}

void lang::make_closure(lang::item_set_t& item_set, const std::vector<lang::prod_rule_t>& prod_rules){
    std::size_t last_size;

    do {
        last_size = item_set.size();
        for (const auto& item : item_set){
            int pos = item.second;  // dot position
            lang::production_t prod = item.first.second;  // production list
            enum lang::Symbol next_symbol = prod[pos];
            
            // Find all productions that start with the next_symbol
            // TODO: See if we can optomize this later
            for (const auto prod_rule : prod_rules){
                if (next_symbol == prod_rule.first){
                    item_set.insert({prod_rule, 0});
                }
            }
        }
    } while (item_set.size() != last_size); // while the item set is changing
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
