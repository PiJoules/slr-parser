#include "parser.h"

static char PRECEDENCE_OVERIDER = '%';

/******** ParseRule **********/

/**
 * Equality between parse rules. Just compare the rule and production.
 */ 
bool parsing::ParseRule::operator==(const ParseRule& other) const {
    return rule == other.rule && production == other.production;
}

/**
 * String representation of this parse rule.
 */
std::string parsing::ParseRule::str() const {
    std::ostringstream s;
    s << rule << " -> ";

    std::size_t len = production.size();
    int end = static_cast<int>(len) - 1;
    for (int i = 0; i < end; ++i){
        s << production[i] << " ";
    }
    if (len){
        s << production.back();
    }

    return s.str();
}

/**
 * Hashing for the ParseRule. Logic is borrowed from python 3.6's tuple hash.
 */ 
std::size_t parsing::ParseRuleHasher::operator()(const ParseRule& parse_rule) const {
    const std::string& rule = parse_rule.rule;
    const std::vector<std::string>& prod = parse_rule.production;
    const std::hash<std::string> str_hasher;
    std::size_t rule_hash = str_hasher(rule);

    std::size_t hash_mult = 1000003;
    std::size_t prod_hash = 0x345678;
    std::size_t len = prod.size();
    for (const std::string& r : prod){
        prod_hash = (prod_hash ^ str_hasher(r)) * hash_mult;
        hash_mult += 82520 + len + len;
    }
    prod_hash += 97531;
    return prod_hash ^ rule_hash;
}


/********* LRItem **********/

bool parsing::LRItem::operator==(const LRItem& other) const {
    return parse_rule == other.parse_rule && pos == other.pos;
}

/**
 * String representation of this lr item.
 */
std::string parsing::LRItem::str() const {
    std::ostringstream s;
    s << parse_rule.rule << " : ";

    // Before dot
    for (std::size_t i = 0; i < pos; ++i){
        s << parse_rule.production[i] << " ";
    }

    s << ". ";

    // After dot 
    int len = static_cast<int>(parse_rule.production.size());
    for (int i = pos; i < len; ++i){
        s << parse_rule.production[i] << " ";
    }

    return s.str();
}

/**
 * Hash will just be the xor between the hash of the parse rule and the position.
 */
std::size_t parsing::LRItemHasher::operator()(const LRItem& lr_item) const {
    ParseRuleHasher hasher;
    return hasher(lr_item.parse_rule) ^ static_cast<std::size_t>(lr_item.pos);
}


/**
 * LRItemSetHasher
 * Logic borrowed from python3.6's frozenset hash
 */ 
static std::size_t _shuffle_bits(std::size_t h){
    return ((h ^ 89869747UL) ^ (h << 16)) * 3644798167UL;
}

std::size_t parsing::LRItemSetHasher::operator()(const LRItemSet& item_set) const {
    std::size_t hash = 0;
    LRItemHasher item_hasher;
    for (const auto& lr_item : item_set){
        hash ^= _shuffle_bits(item_hasher(lr_item));  // entry hashes
    }
    hash ^= item_set.size() * 1927868237UL;  // # of active entrues
    hash = hash * 69069U + 907133923UL;
    return hash;
};

/**
 * String representation of the Action in a Parse Instruction.
 */
std::string parsing::action_str(const ParseInstr::Action& action){
    std::ostringstream s;
    switch (action){
        case ParseInstr::SHIFT: return "shift";
        case ParseInstr::REDUCE: return "reduce";
        case ParseInstr::GOTO: return "goto";
        case ParseInstr::ACCEPT: return "accept";
    }
    return s.str();
}

/**
 * Add a substitute rule that will act as the new top level rule
 */

static void* parse_prime(std::vector<void*>& args, void* data){
    return args.front();
}

std::vector<parsing::ParseRule> parsing::prepend_prime_rule(std::vector<ParseRule> parse_rules){
    std::string old_top_rule = parse_rules.front().rule;
    ParseRule new_top_pr = {
        old_top_rule + "'",  // prime rule
        {old_top_rule}, 
        parse_prime
    };
    parse_rules.insert(parse_rules.begin(), new_top_pr);
    return parse_rules;
}

/**
 * Remove any tokens at the end of each production starting 
 * with the precedence overider character.
 */ 
std::vector<parsing::ParseRule> parsing::trim_overload_tokens(std::vector<ParseRule> parse_rules){
    for (ParseRule& parse_rule : parse_rules){
        std::vector<std::string>& prod = parse_rule.production;
        if (prod.back()[0] == PRECEDENCE_OVERIDER){
            prod.pop_back();
        }
    }
    return parse_rules;
}

/**
 * Initialize a closure from an item set and list of productions.
 * This essentially expands the entire set of lr items by adding other lr items that 
 * start at position zero into this set.
 *
 * repeat
 *     for A->a.Bb in item_set:
 *         item_set.insert(B->.a)
 * until item_set not changing
 */
void parsing::init_closure(LRItemSet& item_set, const std::vector<ParseRule>& parse_rules){
    // For marking which lr items were already inserted
    std::unordered_set<LRItem, LRItemHasher> found_items(item_set.begin(), item_set.end());

    std::size_t i = 0;
    while (i < item_set.size()){
        const LRItem& item = item_set[i];
        std::size_t pos = item.pos;  // dot position
        std::vector<std::string> prod = item.parse_rule.production;  // production list

        // If we have not reached the end of a production
        if (pos < prod.size()){
            std::string next_symbol = prod[pos];
            
            // Find all productions that start with the next_symbol
            // TODO: See if we can optomize this later
            for (const ParseRule parse_rule : parse_rules){
                if (next_symbol == parse_rule.rule){
                    LRItem next_item = {parse_rule, 0};

                    // Be sure to check if we have already found this item before
                    // New items are appended to the end and are always checked
                    if (found_items.find(next_item) == found_items.end()){
                        item_set.push_back(next_item);
                        found_items.insert(next_item);
                    }
                }
            }
        }

        ++i;
    }
}

/**
 * Advance the position of all lr items in an item set and return the new set.
 *
 * s = {}
 * for A->a.Xb in item_set:  # Where X is symbol in this case
 *     s.insert(A->aX.b)
 * return closure(s)
 */ 
parsing::LRItemSet parsing::move_pos(const LRItemSet& item_set, 
                                     const std::string& symbol,
                                     const std::vector<ParseRule>& parse_rules){
    LRItemSet moved_item_set;

    for (const LRItem lr_item : item_set){
        ParseRule parse_rule = lr_item.parse_rule;
        std::vector<std::string>& prod = parse_rule.production;
        std::size_t pos = lr_item.pos;

        if (pos < prod.size()){
            if (prod[pos] == symbol){
                LRItem advanced_item = {parse_rule, pos + 1};
                moved_item_set.push_back(advanced_item);
            }
        }
    }

    // Expand the item set
    init_closure(moved_item_set, parse_rules);

    return moved_item_set;
}

/**
 * Create the canonical collections of the DFA.
 *
 * C = {closure({S'->.S})}
 * repeat 
 *     for item_set in C:
 *         for A->a.Xb in item_set:
 *             C |= move_pos(item_set, X)
 * until C not changing
 * return C
 */ 
parsing::DFA parsing::make_dfa(const std::vector<ParseRule>& parse_rules){
    const ParseRule& entry = parse_rules.front();
    LRItemSet top_item_set = {{entry, 0}};
    init_closure(top_item_set, parse_rules);
    DFA dfa = {top_item_set};

    std::unordered_set<LRItemSet, LRItemSetHasher> found_sets(dfa.begin(), dfa.end());

    std::size_t i = 0;
    while (i < dfa.size()){
        const LRItemSet item_set = dfa[i];  // do copy since this item_set may be edited

        for (const LRItem& lr_item : item_set){
            std::vector<std::string> production = lr_item.parse_rule.production;
            std::size_t pos = lr_item.pos;
            if (pos < production.size()){
                std::string next_symbol = production[pos];
                LRItemSet moved_item_set = move_pos(item_set, next_symbol, parse_rules);

                if (found_sets.find(moved_item_set) == found_sets.end()){
                    dfa.push_back(moved_item_set);
                    found_sets.insert(moved_item_set);
                }
            }
        }

        ++i;
    }

    return dfa;
}

/**
 * Convert the precedence list to a precedence table.
 */
parsing::PrecedenceTable parsing::make_precedence_table(const PrecedenceList& precedence){
    PrecedenceTable prec_table;
    prec_table.reserve(precedence.size());
    for (std::size_t i = 0; i < precedence.size(); ++i){
        const auto& entry = precedence[i];
        enum Associativity assoc = entry.first;
        const auto& tokens = entry.second;
        for (const std::string& tok : tokens){
            prec_table[tok] = {i, assoc};
        }
    }
    return prec_table;
}


/********* Grammar **********/ 

/**
 * A symbol is a terminal if it's in the tokens set.
 */
bool parsing::Grammar::is_terminal(const std::string& symbol) const {
    return tokens_.find(symbol) != tokens_.end();
}

/**
 * Get the token matching to the given instruction and lookahead.
 * Used for determining precedence between instructions for ceertain lookaheads.
 */
std::string parsing::Grammar::token_for_instr(const ParseInstr& instr, const std::string& lookahead) const {
    std::string key_term;
    if (instr.action == ParseInstr::REDUCE){
        // Check for last symbol starting with precedence overrider
        std::size_t rule_num = instr.value;
        const std::vector<std::string>& reduce_prod = parse_rules_with_overloads_[rule_num].production;

        if (reduce_prod.back()[0] == PRECEDENCE_OVERIDER){
            // Use this as the token unstead of the rightmost terminal 
            key_term = reduce_prod.back().substr(1);
        }
        else {
            key_term = rightmost_terminal(reduce_prod);
        }
    }
    else {
        key_term = lookahead;
    }
    return key_term;
}

/**
 * There is a possible parse conflict between two instructions.
 * Compare the instructions and update the parse table for the one with higher precedence.
 * If either are not given in the precedence list, mark it as a parser conflict.
 * Otherwise, only add a conflict if both are reduce.
 * In the case of a shift reduce conflict, the shift is given priority and no conflict 
 * is recorded.
 */
void parsing::Grammar::update_with_precedence(
        const ParseInstr& existing_instr,
        const ParseInstr& new_instr,
        const std::string& lookahead,
        std::size_t state){
    // Check if both are the same first 
    if (existing_instr == new_instr){
        return;
    }

    std::unordered_map<std::string, ParseInstr>& action_table = parse_table_[state];

    // Tterminal key for existing instr
    const std::string key_existing = token_for_instr(existing_instr, lookahead);

    // Terminal key for new instr 
    const std::string key_new = token_for_instr(new_instr, lookahead);

    if (precedence_map_.find(key_existing) != precedence_map_.cend() &&
        precedence_map_.find(key_new) != precedence_map_.cend()){
        // Both have precedence rules. No conflict yet.
        const auto& prec_existing = precedence_map_.at(key_existing);
        const auto& prec_new = precedence_map_.at(key_new);
        if (prec_new.first > prec_existing.first){
            // Take the new action over current
            action_table[key_new] = new_instr;
        }
        else if (prec_new.first < prec_existing.first){
            // Take existing over new. Don't need to do anything.
        }
        else {
            // Same precedence 
            // Take account into associativity if either are shift
            if (new_instr.action == ParseInstr::SHIFT || existing_instr.action == ParseInstr::SHIFT){
                // Find the shift instr
                ParseInstr shift_instr, reduce_instr;
                if (new_instr.action == ParseInstr::SHIFT){
                    shift_instr = new_instr;
                    reduce_instr = existing_instr;
                }
                else {
                    shift_instr = existing_instr;
                    reduce_instr = new_instr;
                }

                // Assign based on associativity
                // If left assoc, reduce. If right assoc, shift.
                if (prec_new.second == LEFT_ASSOC){
                    action_table[lookahead] = reduce_instr;
                }
                else {
                    action_table[lookahead] = shift_instr;
                }
            }
            else {
                // Both are reduce. Cannot resolve this, so add it as a conflict.
                conflicts_.push_back({
                    state,
                    existing_instr,
                    new_instr,
                    lookahead,
                });
            }
        }
    }
    else {
        // Conflict
        conflicts_.push_back({
            state,
            existing_instr,
            new_instr,
            lookahead,
        });
    }
}

/**
 * String representation of the conflict.
 * This belongs to Grammar b/c the conlfict needs to know the rightmost_terminal in its production 
 * which depends on knowing the tokens in the lexer.
 */
std::string parsing::Grammar::conflict_str(const ParseInstr& instr, const std::string lookahead) const {
    std::ostringstream stream;
    std::vector<std::string> prod;
    switch (instr.action){
        case ParseInstr::SHIFT: 
            stream << "shift and go to state " << instr.value << " on lookahead " << lookahead;
            break;
        case ParseInstr::REDUCE: 
            prod = parse_rules_[instr.value].production;
            stream << "reduce using rule " << instr.value << " on terminal " << rightmost_terminal(prod);
            break;
        case ParseInstr::GOTO: 
            stream << "go to state " << instr.value;
            break;
        case ParseInstr::ACCEPT: 
            stream << "accept";
    }
    return stream.str();
}

/**
 * Just iterate backwards through the production until we find the first terminal symbol.
 * Default will just be an empty string.
 */
std::string parsing::Grammar::rightmost_terminal(const std::vector<std::string>& prod) const {
    for (auto it = prod.crbegin(); it != prod.crend(); ++it){
        if (is_terminal(*it)){
            return *it;
        }
    }
    return "";
}

/**
 * Method for initially creating the firsts set for a nonterminal before memoizing.
 */
std::unordered_set<std::string> parsing::Grammar::nonterminal_firsts(const std::string& symbol){
    std::unordered_set<std::string> firsts_set;

    for (const auto& parse_rule : parse_rules_){
        const std::string& rule = parse_rule.rule;
        const std::vector<std::string>& prod = parse_rule.production;
        if (rule == symbol){
            // For each production mapped to rule X 
            // put firsts(Y1) - {e} into firsts(X)
            std::unordered_set<std::string> diff(firsts(prod[0]));
            if (diff.find(parsing::nonterminals::EPSILON) != diff.end()){
                diff.erase(parsing::nonterminals::EPSILON);
            }
            firsts_set.insert(diff.begin(), diff.end());

            // Check for epsilon in each symbol in the prod 
            bool all_have_empty = true;
            for (std::size_t i = 0; i < prod.size()-1; ++i){
                const std::unordered_set<std::string> current_set = firsts(prod[i]);
                if (current_set.find(parsing::nonterminals::EPSILON) != current_set.end()){
                    std::unordered_set<std::string> next_set = firsts(prod[i+1]);
                    if (next_set.find(parsing::nonterminals::EPSILON) != next_set.end()){
                        next_set.erase(parsing::nonterminals::EPSILON);
                    }
                    firsts_set.insert(next_set.begin(), next_set.end());
                }
                else {
                    all_have_empty = false;
                }
            }
            // Check for last one having epsilon
            const std::unordered_set<std::string> last_set = firsts(prod.back());
            if (all_have_empty && last_set.find(parsing::nonterminals::EPSILON) != last_set.end()){
                firsts_set.insert(parsing::nonterminals::EPSILON);
            }
        }
    }

    return firsts_set;
}

/**
 * Constructor for Grammar.
 * This initializes the parse table from the production rules.
 */
parsing::Grammar::Grammar(const std::unordered_set<std::string>& tokens, 
                          const std::vector<ParseRule>& parse_rules, 
                          const PrecedenceList& precedence):
    tokens_(tokens), 
    parse_rules_with_overloads_(prepend_prime_rule(parse_rules)),
    parse_rules_(trim_overload_tokens(parse_rules_with_overloads_)), 
    start_nonterminal_(parse_rules_.front().rule),
    precedence_map_(make_precedence_table(precedence)),
    dfa_(make_dfa(parse_rules_))
{
    const ParseRule& top_parse_rule = parse_rules_.front();
    parse_table_.reserve(dfa_.size());

    std::unordered_map<LRItemSet, std::size_t, LRItemSetHasher> item_set_map;
    item_set_map.reserve(dfa_.size());

    // Map the production rules to the order in which they appear
    std::unordered_map<ParseRule, std::size_t, ParseRuleHasher> parse_rule_map;
    parse_rule_map.reserve(parse_rules_.size());
    for (std::size_t i = 0; i < parse_rules_.size(); ++i){
        parse_rule_map[parse_rules_[i]] = i;
    }

    // Map the item_sets to their final indeces in the map 
    for (std::size_t i = 0; i < dfa_.size(); ++i){
        item_set_map[dfa_[i]] = i;

        std::unordered_map<std::string, ParseInstr> action_map;
        parse_table_[i] = action_map;
    }

    for (std::size_t i = 0; i < dfa_.size(); ++i){
        const LRItemSet& item_set = dfa_[i];
        for (const LRItem& lr_item : item_set){
            const ParseRule& parse_rule = lr_item.parse_rule;
            const std::vector<std::string>& prod = parse_rule.production;
            const std::size_t& pos = lr_item.pos;
            std::unordered_map<std::string, ParseInstr>& action_table = parse_table_[i];
            if (pos < prod.size()){
                // If A -> x . a y and GOTO(I_i, a) == I_j, then ACTION[i, a] = Shift j 
                // If we have a rule where the symbol following the parser position is 
                // a terminal, shift to the jth state which is equivalent to GOTO(I_i, a).
                const std::string& next_symbol = prod[pos];
                const LRItemSet I_j = move_pos(item_set, next_symbol, parse_rules_);
                int j = item_set_map.at(I_j);

                if (is_terminal(next_symbol)){
                    // next_symbol is a token 
                    auto existing_it = action_table.find(next_symbol);
                    ParseInstr shift_instr = {parsing::ParseInstr::Action::SHIFT, j};
                    if (existing_it != action_table.cend()){
                        // Possible action conlfict. Check for precedence 
                        update_with_precedence(existing_it->second, shift_instr, next_symbol, i);
                    }
                    else {
                        // No conflict. Fill with shift
                        action_table[next_symbol] = shift_instr;
                    }
                }
                else {
                    action_table[next_symbol] = {parsing::ParseInstr::Action::GOTO, j};
                }
            }
            else {
                if (parse_rule == top_parse_rule){
                    // Finished whole module; cannot reduce further
                    action_table[lexing::tokens::END] = {parsing::ParseInstr::Action::ACCEPT, 0};
                }
                else {
                    // End of rule; Reduce 
                    // If A -> a ., then ACTION[i, b] = Reduce A -> a for all terminals 
                    // in B -> A . b where b is a terminal
                    int rule_num = parse_rule_map.at(parse_rule);
                    const std::string& rule = parse_rule.rule;
                    ParseInstr instr = {parsing::ParseInstr::Action::REDUCE, rule_num};
                    
                    for (const std::string& follow : follows(rule)){
                        if (action_table.find(follow) != action_table.cend()){
                            // Possible conflict 
                            update_with_precedence(action_table[follow], instr, follow, i);
                        }
                        else {
                            action_table[follow] = instr;
                        }
                    }
                }
            }
        }
    }

    assert(firsts_stack_.empty());
    assert(follows_stack_.empty());
}

/**
 * Pretty print the parse table similar to how ply prints it.
 */
void parsing::Grammar::dump(std::ostream& stream) const {
    stream << "Grammar" << std::endl << std::endl;
    for (std::size_t i = 0; i < parse_rules_with_overloads_.size(); ++i){
        stream << "Rule " << i << ": " << parse_rules_with_overloads_[i].str() << std::endl;
    }
    stream << std::endl;

    // States 
    for (std::size_t i = 0; i < parse_table_.size(); ++i){
        dump_state(i, stream);
    }
    stream << std::endl;

    // Conflicts  
    stream << "Conflicts (" << conflicts_.size() << ")" << std::endl << std::endl;
    for (const ParserConflict& conflict : conflicts_){
        std::size_t state = conflict.state;
        const ParseInstr& chosen = conflict.instr1;
        const ParseInstr& other = conflict.instr2;
        const std::string& lookahead = conflict.lookahead;

        const ParseInstr::Action& act1 = chosen.action;
        const ParseInstr::Action& act2 = other.action;
        stream << action_str(act1) << "/" << action_str(act2) << " conflict in state " << state
               << " (defaulting to " 
               << action_str(act1) << ")" << std::endl;
        stream << "- " << conflict_str(chosen, lookahead) << std::endl;
        stream << "- " << conflict_str(other, lookahead) << std::endl;
    }
}

/**
 * Dump a state in the grammar, the state being an LRItemSet.
 */
void parsing::Grammar::dump_state(std::size_t state, std::ostream& stream) const {
    stream << "state " << state << std::endl << std::endl;

    // Print item sets
    const LRItemSet& item_set = dfa_[state];
    for (const LRItem& lr_item : item_set){
        stream << "\t" << lr_item.str() << std::endl;
    }
    stream << std::endl;

    // Print parse instructions
    const auto& action_map = parse_table_.at(state);
    for (auto it = action_map.cbegin(); it != action_map.cend(); ++it){
        const std::string& symbol = it->first;
        const ParseInstr& instr = it->second;
        const ParseInstr::Action& action = instr.action;

        int val = instr.value;
        stream << "\t" << symbol << "\t\t";

        switch (action){
            case ParseInstr::SHIFT:
                stream << "shift and go to state ";
                break;
            case ParseInstr::REDUCE:
                stream << "reduce using rule ";
                break;
            case ParseInstr::GOTO:
                stream << "goto state ";
                break;
            case ParseInstr::ACCEPT:
                stream << "accept ";
                break;
        }
        stream << val << std::endl;
    }
    stream << std::endl;
}

/**
 * The first and follow sets are found lazily, but memoized.
 */
std::unordered_set<std::string> parsing::Grammar::firsts(const std::string& symbol){
    if (firsts_map_.find(symbol) != firsts_map_.end()){
        return firsts_map_[symbol];
    }

    if (is_terminal(symbol) || symbol == parsing::nonterminals::EPSILON){
        firsts_map_[symbol] = {symbol};
        return firsts_map_[symbol];
    }
    else {
        if (firsts_stack_.find(symbol) == firsts_stack_.end()){
            firsts_stack_.insert(symbol);
            firsts_map_[symbol] = nonterminal_firsts(symbol);
            firsts_stack_.erase(symbol);
            return firsts_map_[symbol];
        }
        else {
            std::unordered_set<std::string> s;
            return s;
        }
    }
}

std::unordered_set<std::string> parsing::Grammar::follows(const std::string& symbol){
    // If the symbol is already in the follows map, return it
    if (follows_map_.find(symbol) != follows_map_.end()){
        return follows_map_[symbol];
    }

    // Return nothing if we have already seen this symbol in a recursive call
    if (follows_stack_.find(symbol) != follows_stack_.end()){
        std::unordered_set<std::string> s;
        return s;
    }

    // Add for the first time
    follows_stack_.insert(symbol);

    // Initialize the set with the symbol if it is the starting nonterminal
    std::unordered_set<std::string> follows_set;
    if (symbol == start_nonterminal_){
        follows_set.insert(lexing::tokens::END);
    }

    // Find the productions with this symbol on the RHS 
    for (const auto& parse_rule : parse_rules_){
        // For each rule and production
        const std::string& rule = parse_rule.rule;
        const std::vector<std::string>& prod = parse_rule.production;

        // Check for other symbols that follow this one in a production
        for (std::size_t i = 0; i < prod.size()-1; ++i){
            if (prod[i] == symbol){
                const std::unordered_set<std::string> next_set = firsts(prod[i+1]);
                std::unordered_set<std::string> next_without_eps(next_set);
                if (next_without_eps.find(parsing::nonterminals::EPSILON) != next_without_eps.end()){
                    next_without_eps.erase(parsing::nonterminals::EPSILON);
                }

                // Add to the follows set 
                // follows_set |= self.firsts(prod[i+1]) - {EPSILON}
                follows_set.insert(next_without_eps.begin(), next_without_eps.end());

                // Add follows of this rule if the firsts contains epsilon
                // if EPSILON in self.firsts(prod[i+1)
                if (next_set.find(parsing::nonterminals::EPSILON) != next_set.end()){
                    // follows_set |= self.follows(rule)
                    std::unordered_set<std::string> rule_follows = follows(rule);
                    follows_set.insert(rule_follows.begin(), rule_follows.end());
                }
            }
        }
        if (prod.back() == symbol){
            // Add the follows(rule) if this symbol is the last one in the production
            std::unordered_set<std::string> rule_follows = follows(rule);
            follows_set.insert(rule_follows.begin(), rule_follows.end());
        }
    }

    // Memoize and remove from stack 
    follows_map_[symbol] = follows_set;
    follows_stack_.erase(symbol);

    return follows_set;
}


/**
 * Grammar getters
 */
const parsing::ParseTable& parsing::Grammar::parse_table() const { return parse_table_; }
const std::vector<parsing::ParseRule>& parsing::Grammar::parse_rules() const { return parse_rules_; }
const std::vector<parsing::ParserConflict>& parsing::Grammar::conflicts() const { return conflicts_; }
const std::unordered_map<std::string, std::unordered_set<std::string>>& parsing::Grammar::firsts() const { return firsts_map_; };
const std::unordered_map<std::string, std::unordered_set<std::string>>& parsing::Grammar::follows() const { return follows_map_; };



/**************** Parser ************/ 

void* parsing::NodeVisitor::visit(Node* node){
    return node->accept(*this);
}

/**
 * All terminal symbols on the stack have the same precedence and associativity.
 * Reduce depending on the type of associativity.
 */
void parsing::Parser::reduce(
        const ParseRule& parse_rule, 
        std::vector<lexing::LexToken>& symbol_stack,
        std::vector<void*>& node_stack,
        std::vector<std::size_t>& state_stack,
        void* data){
    const std::string& rule = parse_rule.rule;
    const std::vector<std::string>& prod = parse_rule.production;
    const ParseCallback func = parse_rule.callback;
    
    lexing::LexToken rule_token = {rule,"",0,0,0};

    // Note: the symbol stack and production may not be the same length, but the 
    // symbol stack and node stack will always be the same size
    assert(node_stack.size() == symbol_stack.size());

    auto start = node_stack.begin() + node_stack.size() - prod.size();
    void* result_node;

    if (func){
        std::vector<void*> slice(start, node_stack.end());
        result_node = func(slice, data);
    }
    else {
        // Otherwise, add the wrapper for the rule token
        result_node = new lexing::LexToken(rule_token);
    }

    node_stack.erase(start, node_stack.end());
    node_stack.push_back(result_node);
    
    state_stack.erase(state_stack.end()-prod.size(), state_stack.end());
    symbol_stack.erase(symbol_stack.end()-prod.size(), symbol_stack.end());
    symbol_stack.push_back(rule_token);

    // Next instruction will be GOTO
    ParseInstr next_instr = get_instr(state_stack.back(), rule_token);
    assert(next_instr.action == ParseInstr::GOTO);
    state_stack.push_back(next_instr.value);

    assert(node_stack.size() == symbol_stack.size());
}


/**
 * Lookup of a parse instruction in the parse table with possible parse error getting raised.
 */
const parsing::ParseInstr& parsing::Parser::get_instr(std::size_t state, const lexing::LexToken& lookahead){
    const ParseTable& parse_table = grammar_.parse_table();
    if (parse_table.at(state).find(lookahead.symbol) == parse_table.at(state).cend()){
        throw ParseError(*this, state, lookahead);
    }
    return parse_table.at(state).at(lookahead.symbol);
}


/**
 * Constructors
 */
parsing::Parser::Parser(lexing::Lexer& lexer, const Grammar& grammar): 
    lexer_(lexer), grammar_(grammar){}

parsing::Parser::Parser(lexing::Lexer& lexer, const std::vector<ParseRule>& parse_rules,
                        const PrecedenceList& precedence):
    lexer_(lexer), 
    grammar_(Grammar(keys(lexer.tokens()), parse_rules, precedence)){}


/**
 * The actual parsing.
 */
void* parsing::Parser::parse(const std::string& code, void* data){
    // This language is defined such that all statements must end with a newline 
    std::string code_cpy = code + "\n";

    // Input the string
    lexer_.input(code_cpy);

    std::vector<std::size_t> state_stack;

    // Add the initial state number
    state_stack.push_back(0);

    std::vector<lexing::LexToken> symbol_stack;
    std::vector<void*> node_stack;

    lexing::LexToken lookahead = lexer_.token(data);
    const std::vector<ParseRule>& parse_rules = grammar_.parse_rules();

    while (1){
        std::size_t state = state_stack.back();

#ifdef DEBUG
        // Dump the stack  
        std::cerr << "stack: ";
        for (const auto& symbol : symbol_stack){
            std::cerr << symbol.symbol << ", ";
        }
        std::cerr << std::endl;
#endif

        lexing::LexToken* stack_token;
        const ParseInstr& instr = get_instr(state, lookahead);

        switch (instr.action){
            case ParseInstr::SHIFT:
#ifdef DEBUG
                std::cerr << "Shift " << lookahead.symbol << " and goto state " << instr.value << std::endl;
#endif
                // Add the next state to its stack and the lookahead to the tokens stack
                state_stack.push_back(instr.value);
                symbol_stack.push_back(lookahead);

                // Copy the lookahead data
                stack_token = new lexing::LexToken(lookahead);
                node_stack.push_back(stack_token);

                lookahead = lexer_.token(data);
                break;
            case ParseInstr::REDUCE:
#ifdef DEBUG
                std::cerr << "Reduce using rule " << instr.value << std::endl;
#endif

                // Pop from the states stack and replace the rules in the tokens stack 
                // with the reduce rule
                reduce(parse_rules[instr.value], symbol_stack, node_stack, state_stack, data);
                break;
            case ParseInstr::ACCEPT:
#ifdef DEBUG
                std::cerr << "Accept " << instr.value << " (" << symbol_stack.back().symbol << ")" << std::endl;
#endif

                // Reached end
                assert(symbol_stack.size() == 1);
                assert(node_stack.size() == 1);
                return node_stack.front();
            case ParseInstr::GOTO:
                // Should not actually end up here since gotos are handled in reduce 
                // Though you may end up here if you have found a token that was not declared as a terminal 
                std::string err = "Check if '" + lookahead.value + "' matches the regex for a valid token.";
                throw std::runtime_error(err);
        }
    }
}


/**
 * Getters
 */
const parsing::Grammar& parsing::Parser::grammar() const { return grammar_; }


/************ ParseError ************/

parsing::ParseError::ParseError(const Parser& parser, const std::size_t state, 
                                const lexing::LexToken& lookahead):
    std::runtime_error("Parser error"), parser_(parser), state_(state),
    lookahead_(lookahead){}

const char* parsing::ParseError::what() const throw(){
    std::ostringstream err;
    err << ": Unable to handle lookahead '" << lookahead_.symbol << "' in state " << state_ 
        << ". Line " << lookahead_.lineno << ", col " << lookahead_.colno << "."
        << std::endl << std::endl;
    parser_.grammar().dump_state(state_, err);
    return err.str().c_str();
}
