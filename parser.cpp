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
    return static_cast<int>(symbol) >= 0;
}

bool lang::is_rule(const enum lang::Symbol& symbol){
    return !is_token(symbol);
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
void lang::init_closure(lang::item_set_t& item_set, const std::vector<lang::prod_rule_t>& prod_rules){
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
    init_closure(moved_item_set, prod_rules);
    return item_set_t(moved_item_set);
}

/**
 * Create the canonical collections of the DFA.
 */ 
void lang::init_dfa(lang::dfa_t& dfa, const std::vector<lang::prod_rule_t>& prod_rules){
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

void lang::Parser::input(const std::string& code){
    lexer.input(code);
}

/**
 * Initialize the parse table from the production rules.
 */
lang::Parser::Parser(const std::vector<prod_rule_t>& prod_rules):
    prod_rules_(prod_rules){
    const auto& entry = prod_rules_.front();
    item_set_t item_set = {{entry, 0}};
    init_closure(item_set, prod_rules_);
    dfa_t dfa = {item_set};
    init_dfa(dfa, prod_rules_);
    init_parse_table(dfa);
}

void lang::Parser::init_parse_table(const dfa_t& dfa){
    const auto& top_prod_rule = prod_rules_.front();
    parse_table_.reserve(dfa.size());

    // Map the item_sets to their final indeces in the map 
    std::size_t i = 0;
    for (const auto& item_set : dfa){
        std::unordered_map<enum Symbol, ParseInstr, SymbolHasher> action_map;
        parse_table_[i] = action_map;
        item_set_map_[item_set] = i;
        i++;
    }

    // Map the production rules to the order in which they appear
    for (i = 0; i < prod_rules_.size(); ++i){
        prod_rule_map_[prod_rules_[i]] = i;
    }

    i = 0;
    for (const auto& item_set : dfa){
        for (const auto& lr_item : item_set){
            const auto& prod_rule = lr_item.first;
            const auto& prod = prod_rule.second;
            const std::size_t& pos = lr_item.second;
            if (pos < prod.size()){
                // If A -> x . a y and GOTO(I_i, a) == I_j, then ACTION[i, a] = Shift j 
                // If we have a rule where the symbol following the parser position is 
                // a terminal, shift to the jth state which is equivalent to GOTO(I_i, a).
                const auto& next_symbol = prod[pos];
                const auto I_j = move_pos(item_set, next_symbol, prod_rules_);
                int j = item_set_map_[I_j];

                if (is_token(next_symbol)){
                    parse_table_[i][next_symbol] = {lang::ParseInstr::Action::SHIFT, j};
                }
                else {
                    parse_table_[i][next_symbol] = {lang::ParseInstr::Action::GOTO, j};
                }
            }
            else {
                if (prod_rule == top_prod_rule){
                    // Finished whole module; cannot reduce further
                    parse_table_[i][eof_tok] = {lang::ParseInstr::Action::ACCEPT, 0};
                }
                else {
                    // End of rule; Reduce 
                    const auto& last_symbol = prod.back();
                    int rule_num = prod_rule_map_[prod_rule];
                    parse_table_[i][last_symbol] = {lang::ParseInstr::Action::REDUCE, rule_num};
                }
            }
        }
        i++;
    }
}

/**
 * Pretty print the parse table similar to how ply prints it.
 */
void lang::Parser::dump_grammar(std::ostream& stream) const {
    item_set_t item_sets[item_set_map_.size()];
    for (auto it = item_set_map_.cbegin(); it != item_set_map_.cend(); ++it){
        item_sets[it->second] = it->first;
    }
        
    // Grammar 
    stream << "Grammar" << std::endl << std::endl;
    for (std::size_t i = 0; i < prod_rules_.size(); ++i){
        stream << "Rule " << i << ": " << str(prod_rules_[i]) << std::endl;
    }
    stream << std::endl;

    // States 
    for (std::size_t i = 0; i < parse_table_.size(); ++i){
        stream << "state " << i << std::endl << std::endl;

        // Print item sets
        const auto& item_set = item_sets[i];
        for (const auto& lr_item : item_set){
            stream << "\t" << lang::str(lr_item) << std::endl;
        }
        stream << std::endl;

        // Print parse instructions
        const auto& action_map = parse_table_.at(i);
        for (auto it = action_map.cbegin(); it != action_map.cend(); ++it){
            const auto& symbol = it->first;
            const auto& instr = it->second;
            const auto& action = instr.action;

            if (action == lang::ParseInstr::SHIFT || action == lang::ParseInstr::REDUCE){
                int val = instr.value;
                stream << "\t" << str(symbol) << "\t\t";

                if (action == lang::ParseInstr::SHIFT){
                    stream << "shift and go to state ";
                }
                else {
                    stream << "reduce using rule ";
                }
                stream << val << std::endl;
            }
        }
        stream << std::endl;
    }
}
