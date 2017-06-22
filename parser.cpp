#include "lang.h"

/**
 * Borrowed from python3.6's tuple hash
 */
std::size_t lang::ProdRuleHasher::operator()(const prod_rule_t& prod_rule) const {
    std::string rule = prod_rule.first;
    production_t prod = prod_rule.second;
    std::hash<std::string> str_hasher;
    std::size_t rule_hash = str_hasher(rule);

    std::size_t hash_mult = _HASH_MULTIPLIER;
    std::size_t prod_hash = 0x345678;
    std::size_t len = prod.size();
    for (std::string& r : prod){
        prod_hash = (prod_hash ^ str_hasher(r)) * hash_mult;
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
                std::string next_symbol = prod[pos];
                
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
                                const std::string& symbol,
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

void lang::Parser::init_precedence(const precedence_t& precedence){
    precedence_map_.reserve(precedence.size());
    for (std::size_t i = 0; i < precedence.size(); ++i){
        const auto& entry = precedence[i];
        enum Associativity assoc = entry.first;
        const auto& tokens = entry.second;
        for (const std::string& tok : tokens){
            precedence_map_[tok] = {i, assoc};
        }
    }
}

/**
 * Initialize the parse table from the production rules.
 */
lang::Parser::Parser(Lexer lexer, const std::vector<prod_rule_t>& prod_rules, 
                     const precedence_t& precedence):
    lexer(lexer),
    prod_rules_(prod_rules)
{
    init_precedence(precedence);

    const auto& entry = prod_rules_.front();
    item_set_t item_set = {{entry, 0}};
    init_closure(item_set, prod_rules_);
    dfa_t dfa = {item_set};
    init_dfa(dfa, prod_rules_);
    init_parse_table(dfa);
}

bool lang::Parser::is_terminal(const std::string& symbol) const {
    return lexer.tokens().find(symbol) != lexer.tokens().end();
}

void lang::Parser::init_parse_table(const dfa_t& dfa){
    const auto& top_prod_rule = prod_rules_.front();
    parse_table_.reserve(dfa.size());

    // Map the item_sets to their final indeces in the map 
    std::size_t i = 0;
    for (const auto& item_set : dfa){
        std::unordered_map<std::string, ParseInstr> action_map;
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
            auto& action_table = parse_table_[i];
            if (pos < prod.size()){
                // If A -> x . a y and GOTO(I_i, a) == I_j, then ACTION[i, a] = Shift j 
                // If we have a rule where the symbol following the parser position is 
                // a terminal, shift to the jth state which is equivalent to GOTO(I_i, a).
                const auto& next_symbol = prod[pos];
                const auto I_j = move_pos(item_set, next_symbol, prod_rules_);
                int j = item_set_map_[I_j];

                if (is_terminal(next_symbol)){
                    // next_symbol is a token 
                    auto existing_it = action_table.find(next_symbol);
                    ParseInstr instr = {lang::ParseInstr::Action::SHIFT, j};
                    if (existing_it != action_table.cend()){
                        // Shift reduce conflict. Check for precedence.
                        conflicts_.push_back({
                            instr,
                            existing_it->second,
                            next_symbol,
                        });
                    }
                    // Override existing with shift
                    action_table[next_symbol] = instr;
                }
                else {
                    action_table[next_symbol] = {lang::ParseInstr::Action::GOTO, j};
                }
            }
            else {
                if (prod_rule == top_prod_rule){
                    // Finished whole module; cannot reduce further
                    action_table[tokens::END] = {lang::ParseInstr::Action::ACCEPT, 0};
                }
                else {
                    // End of rule; Reduce 
                    // If A -> a ., then ACTION[i, b] = Reduce A -> a for all terminals 
                    // in B -> A . b where b is a terminal
                    int rule_num = prod_rule_map_[prod_rule];
                    const std::string& rule = prod_rule.first;
                    for (const prod_rule_t& pr : prod_rules_){
                        const production_t& other_prod = pr.second;
                        // Only need to iterate up to the second to last symbol 
                        for (std::size_t j = 0; j < other_prod.size()-1; ++j){
                            const std::string& current_prod = other_prod[j];
                            const std::string& next_prod = other_prod[j+1];
                            if (current_prod == rule && is_terminal(next_prod)){
                                auto existing_it = action_table.find(next_prod);
                                ParseInstr instr = {lang::ParseInstr::Action::REDUCE, rule_num};
                                if (existing_it != action_table.cend()){
                                    // Reduce reduce conflict
                                    conflicts_.push_back({
                                        existing_it->second,
                                        instr,
                                        next_prod,
                                    });
                                }
                                else {
                                    // Precedence will go to the existing one
                                    action_table[next_prod] = instr;
                                }
                            }
                        }
                    }
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
                stream << "\t" << symbol << "\t\t";

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
    stream << std::endl;

    // Conflicts  
    stream << "Conflicts" << std::endl << std::endl;
    for (const ParserConflict& conflict : conflicts_){
        const ParseInstr& chosen = conflict.instr1;
        const ParseInstr& other = conflict.instr2;
        const std::string& lookahead = conflict.lookahead;

        const ParseInstr::Action& act1 = chosen.action;
        const ParseInstr::Action& act2 = other.action;
        stream << str(act1) << "/" << str(act2) << " conflict for lookahead '" << lookahead << "'. Resolved as " << str(chosen);
    }
}
