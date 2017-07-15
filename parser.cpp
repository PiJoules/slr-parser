#include "parser.h"

/**
 * Initialize a closure from an item set and list of productions.
 */
void parsing::init_closure(parsing::item_set_t& item_set, const std::vector<parsing::ParseRule>& prod_rules){
    std::size_t last_size;

    do {
        last_size = item_set.size();
        for (const auto& item : item_set){
            std::size_t pos = item.second;  // dot position
            std::vector<std::string> prod = item.first.production;  // production list
            if (pos < prod.size()){
                std::string next_symbol = prod[pos];
                
                // Find all productions that start with the next_symbol
                // TODO: See if we can optomize this later
                for (const auto prod_rule : prod_rules){
                    if (next_symbol == prod_rule.rule){
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
parsing::item_set_t parsing::move_pos(const parsing::item_set_t& item_set, 
                                const std::string& symbol,
                                const std::vector<parsing::ParseRule>& prod_rules){
    item_set_t moved_item_set;
    for (const auto lr_item : item_set){
        auto prod_rule = lr_item.first;
        const std::vector<std::string>& prod = prod_rule.production;
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
void parsing::init_dfa(parsing::dfa_t& dfa, const std::vector<parsing::ParseRule>& prod_rules){
    std::size_t last_size;
    do {
        last_size = dfa.size();
        for (const item_set_t& item_set : dfa){
            for (const lr_item_t& lr_item : item_set){
                auto production = lr_item.first.production;
                std::size_t pos = lr_item.second;
                if (pos < production.size()){
                    auto next_symbol = production[pos];
                    auto moved_item_set = parsing::move_pos(item_set, next_symbol, prod_rules);
                    dfa.insert(moved_item_set);
                }
            }
        }
    } while (dfa.size() != last_size);  // while dfa did not change
}

void parsing::Parser::init_precedence(const precedence_t& precedence){
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

static void* parse_prime(std::vector<void*>& args, void* data){
    return args.front();
}

/**
 * Initialize the parse table from the production rules.
 */
parsing::Parser::Parser(lexing::Lexer& lexer, const std::vector<ParseRule>& prod_rules, 
                        const precedence_t& precedence):
    lexer_(lexer)
{
    prod_rules_ = prod_rules;

    // Add new top level rule 
    std::string old_top_rule = prod_rules_.front().rule;
    ParseRule new_top_pr = {
        old_top_rule + "'",  // prime rule
        {old_top_rule}, 
        parse_prime
    };
    prod_rules_.insert(prod_rules_.begin(), new_top_pr);

    start_nonterminal_ = prod_rules_.front().rule;
    init_precedence(precedence);

    const auto& entry = prod_rules_.front();
    top_item_set_ = {{entry, 0}};
    init_closure(top_item_set_, prod_rules_);
    dfa_t dfa = {top_item_set_};
    init_dfa(dfa, prod_rules_);
    init_parse_table(dfa);
}

bool parsing::Parser::is_terminal(const std::string& symbol) const {
    return lexer_.tokens().find(symbol) != lexer_.tokens().end();
}

const std::vector<parsing::ParserConflict>& parsing::Parser::conflicts() const {
    return conflicts_;
}

void parsing::Parser::init_parse_table(const dfa_t& dfa){
    const auto& top_prod_rule = prod_rules_.front();
    parse_table_.reserve(dfa.size());
    item_set_map_.reserve(dfa.size());

    // Map the item_sets to their final indeces in the map 
    std::size_t i = 0;
    for (const auto& item_set : dfa){
        std::unordered_map<std::string, ParseInstr> action_map;
        parse_table_[i] = action_map;
        item_set_map_[item_set] = i;
        ++i;
    }

    // Map the production rules to the order in which they appear
    for (i = 0; i < prod_rules_.size(); ++i){
        prod_rule_map_[prod_rules_[i]] = i;
    }

    i = 0;
    for (const auto& item_set : dfa){
        for (const auto& lr_item : item_set){
            const auto& prod_rule = lr_item.first;
            const auto& prod = prod_rule.production;
            const std::size_t& pos = lr_item.second;
            auto& action_table = parse_table_[i];
            if (pos < prod.size()){
                // If A -> x . a y and GOTO(I_i, a) == I_j, then ACTION[i, a] = Shift j 
                // If we have a rule where the symbol following the parser position is 
                // a terminal, shift to the jth state which is equivalent to GOTO(I_i, a).
                const auto& next_symbol = prod[pos];
                const auto I_j = move_pos(item_set, next_symbol, prod_rules_);
                int j = item_set_map_.at(I_j);

                if (is_terminal(next_symbol)){
                    // next_symbol is a token 
                    auto existing_it = action_table.find(next_symbol);
                    ParseInstr shift_instr = {parsing::ParseInstr::Action::SHIFT, j};
                    if (existing_it != action_table.cend()){
                        // Possible action conlfict. Check for precedence 
                        check_precedence(existing_it->second, shift_instr, next_symbol, action_table);
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
                if (prod_rule == top_prod_rule){
                    // Finished whole module; cannot reduce further
                    action_table[lexing::tokens::END] = {parsing::ParseInstr::Action::ACCEPT, 0};
                }
                else {
                    // End of rule; Reduce 
                    // If A -> a ., then ACTION[i, b] = Reduce A -> a for all terminals 
                    // in B -> A . b where b is a terminal
                    int rule_num = prod_rule_map_.at(prod_rule);
                    const std::string& rule = prod_rule.rule;
                    ParseInstr instr = {parsing::ParseInstr::Action::REDUCE, rule_num};
                    
                    for (const std::string& follow : follows(rule)){
                        if (action_table.find(follow) != action_table.cend()){
                            // Possible conflict 
                            check_precedence(action_table[follow], instr, follow, action_table);
                        }
                        else {
                            action_table[follow] = instr;
                        }
                    }
                }
            }
        }
        ++i;
    }
}

std::string parsing::Parser::key_for_instr(const ParseInstr& instr, const std::string& lookahead) const {
    std::string key_term;
    if (instr.action == ParseInstr::REDUCE){
        std::size_t rule_num = instr.value;
        const std::vector<std::string>& reduce_prod = prod_rules_[rule_num].production;
        key_term = rightmost_terminal(reduce_prod);
    }
    else {
        key_term = lookahead;
    }
    return key_term;
}

void parsing::Parser::check_precedence(
        const ParseInstr& existing_instr,
        const ParseInstr& new_instr,
        const std::string& lookahead,
        std::unordered_map<std::string, ParseInstr>& action_table){
    // Possible action conlfict. Check for precedence 
    // Tterminal key for existing instr
    const std::string key_existing = key_for_instr(existing_instr, lookahead);

    // Terminal key for new instr 
    const std::string key_new = key_for_instr(new_instr, lookahead);

    if (precedence_map_.find(key_existing) != precedence_map_.cend() &&
        precedence_map_.find(key_new) != precedence_map_.cend()){
        // Both have precedence rules. No conflict
        const auto& prec_existing = precedence_map_[key_existing];
        const auto& prec_new = precedence_map_[key_new];
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
            existing_instr,
            new_instr,
            lookahead,
        });
    }
}

void parsing::Parser::dump_state(std::size_t state, std::ostream& stream) const {
    item_set_t item_sets[item_set_map_.size()];
    for (auto it = item_set_map_.cbegin(); it != item_set_map_.cend(); ++it){
        item_sets[it->second] = it->first;
    }
        
    stream << "state " << state << std::endl << std::endl;

    // Print item sets
    const auto& item_set = item_sets[state];
    for (const auto& lr_item : item_set){
        stream << "\t" << parsing::str(lr_item) << std::endl;
    }
    stream << std::endl;

    // Print parse instructions
    const auto& action_map = parse_table_.at(state);
    for (auto it = action_map.cbegin(); it != action_map.cend(); ++it){
        const auto& symbol = it->first;
        const auto& instr = it->second;
        const auto& action = instr.action;

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
 * Pretty print the parse table similar to how ply prints it.
 */
void parsing::Parser::dump_grammar(std::ostream& stream) const {
    // Grammar 
    stream << "Grammar" << std::endl << std::endl;
    for (std::size_t i = 0; i < prod_rules_.size(); ++i){
        stream << "Rule " << i << ": " << str(prod_rules_[i]) << std::endl;
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
        const ParseInstr& chosen = conflict.instr1;
        const ParseInstr& other = conflict.instr2;
        const std::string& lookahead = conflict.lookahead;

        const ParseInstr::Action& act1 = chosen.action;
        const ParseInstr::Action& act2 = other.action;
        stream << str(act1) << "/" << str(act2) << " conflict (defaulting to " << str(act1)
               << ")" << std::endl;
        stream << "- " << conflict_str(chosen, lookahead) << std::endl;
        stream << "- " << conflict_str(other, lookahead) << std::endl;
    }
}

std::string parsing::Parser::conflict_str(const ParseInstr& instr, const std::string lookahead) const {
    std::ostringstream stream;
    std::vector<std::string> prod;
    switch (instr.action){
        case ParseInstr::SHIFT: 
            stream << "shift and go to state " << instr.value << " on lookahead " << lookahead;
            break;
        case ParseInstr::REDUCE: 
            prod = prod_rules_[instr.value].production;
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

std::string parsing::Parser::rightmost_terminal(const std::vector<std::string>& prod) const {
    for (auto it = prod.crbegin(); it != prod.crend(); ++it){
        if (is_terminal(*it)){
            return *it;
        }
    }
    return "";
}

/**
 * All terminal symbols on the stack have the same precedence and associativity.
 * Reduce depending on the type of associativity.
 */
void parsing::Parser::reduce(
        const ParseRule& prod_rule, 
        std::vector<lexing::LexToken>& symbol_stack,
        std::vector<void*>& node_stack,
        std::vector<std::size_t>& state_stack,
        void* data){
    const std::string& rule = prod_rule.rule;
    const std::vector<std::string>& prod = prod_rule.production;
    const ParseCallback func = prod_rule.callback;
    
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
        result_node = new LexTokenWrapper(rule_token);
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

const parsing::ParseInstr& parsing::Parser::get_instr(std::size_t state, const lexing::LexToken& lookahead){
    if (parse_table_[state].find(lookahead.symbol) == parse_table_[state].cend()){
        // Parse error
        std::ostringstream err;
        err << "Unable to handle lookahead '" << lookahead.symbol << "' in state " << state 
            << ". Line " << lookahead.lineno << ", col " << lookahead.colno << "."
            << std::endl << std::endl;
        dump_state(state, err);
        throw std::runtime_error(err.str());
    }
    return parse_table_[state][lookahead.symbol];
}

/**
 * The actual parsing.
 */
void* parsing::Parser::parse(const std::string& code, void* data){
    // This language is defined such that all statements must end with a newline 
    std::string code_cpy = code + "\n";

    // Input the string
    lexer_.input(code_cpy);

    std::vector<std::size_t> state_stack;
    state_stack.push_back(item_set_map_.at(top_item_set_));

    std::vector<lexing::LexToken> symbol_stack;
    std::vector<void*> node_stack;

    std::cerr << "called token" << std::endl;
    lexing::LexToken lookahead = lexer_.token(data);
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

        LexTokenWrapper* token_wrapper;
        const ParseInstr& instr = get_instr(state, lookahead);

        switch (instr.action){
            case ParseInstr::SHIFT:
#ifdef DEBUG
                std::cerr << "Shift " << lookahead.symbol << " and goto state " << instr.value << std::endl;
#endif
                // Add the next state to its stack and the lookahead to the tokens stack
                state_stack.push_back(instr.value);
                symbol_stack.push_back(lookahead);

                token_wrapper = new LexTokenWrapper(lookahead);
                node_stack.push_back(token_wrapper);

                std::cerr << "called token" << std::endl;
                lookahead = lexer_.token(data);
                break;
            case ParseInstr::REDUCE:
#ifdef DEBUG
                std::cerr << "Reduce using rule " << instr.value << std::endl;
#endif

                // Pop from the states stack and replace the rules in the tokens stack 
                // with the reduce rule
                reduce(prod_rules_[instr.value], symbol_stack, node_stack, state_stack, data);
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
                break;
        }
    }
}

std::unordered_set<std::string> parsing::Parser::firsts(const std::string& symbol){
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
            firsts_map_[symbol] = make_nonterminal_firsts(symbol);
            firsts_stack_.erase(symbol);
            return firsts_map_[symbol];
        }
        else {
            std::unordered_set<std::string> s;
            return s;
        }
    }
}

std::unordered_set<std::string> parsing::Parser::follows(const std::string& symbol){
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
    for (const auto& prod_rule : prod_rules_){
        // For each rule and production
        const std::string& rule = prod_rule.rule;
        const std::vector<std::string>& prod = prod_rule.production;

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
 * Method for initially creating the firsts set for a nonterminal before memoizing.
 */
std::unordered_set<std::string> parsing::Parser::make_nonterminal_firsts(const std::string& symbol){
    std::unordered_set<std::string> firsts_set;

    for (const auto& prod_rule : prod_rules_){
        const std::string& rule = prod_rule.rule;
        const std::vector<std::string>& prod = prod_rule.production;
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

const std::unordered_set<std::string>& parsing::Parser::firsts_stack() const {
    return firsts_stack_;
}

const std::unordered_set<std::string>& parsing::Parser::follows_stack() const {
    return follows_stack_;
}

std::string parsing::str(const std::vector<std::string>& production){
    std::ostringstream s;
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

std::string parsing::str(const ParseRule& rule){
    std::ostringstream s;
    s << rule.rule << " -> " << str(rule.production);
    return s.str();
}

std::string parsing::str(const lr_item_t& lr_item){
    ParseRule prod_rule = lr_item.first;
    int pos = lr_item.second;
    std::ostringstream s;
    s << prod_rule.rule << " : ";

    // Before dot
    for (int i = 0; i < pos; ++i){
        s << prod_rule.production[i] << " ";
    }

    s << ". ";

    // After dot 
    int len = static_cast<int>(prod_rule.production.size());
    for (int i = pos; i < len; ++i){
        s << prod_rule.production[i] << " ";
    }

    return s.str();
}

std::string parsing::str(const ParseInstr::Action& action){
    switch (action){
        case ParseInstr::SHIFT: return "shift";
        case ParseInstr::REDUCE: return "reduce";
        case ParseInstr::GOTO: return "goto";
        case ParseInstr::ACCEPT: return "accept";
        default: return "";
    }
}


/********** Nodes ************/

std::string parsing::Node::str() const {
    std::vector<std::string> node_lines = lines();
    if (node_lines.empty()){
        return "";
    }

    std::ostringstream s(node_lines.front());
    for (auto it = node_lines.begin()+1; it < node_lines.end(); ++it){
        s << std::endl << *it;
    }
    return s.str();
}

parsing::LexTokenWrapper::LexTokenWrapper(const lexing::LexToken& token): token_(token){}

lexing::LexToken parsing::LexTokenWrapper::token() const { return token_; }

std::vector<std::string> parsing::LexTokenWrapper::lines() const { 
    std::vector<std::string> v = {token_.value};
    return v;
}


/************ Hashes *************/

// Borrowed from python hash
static std::size_t _HASH_MULTIPLIER = 1000003;

/**
 * Equality between parse rules
 */ 
bool parsing::ParseRule::operator==(const ParseRule& other) const {
    return this->rule == other.rule && this->production == other.production;
}

/**
 * Borrowed from python3.6's tuple hash
 */
std::size_t parsing::ProdRuleHasher::operator()(const ParseRule& prod_rule) const {
    const std::string& rule = prod_rule.rule;
    const std::vector<std::string>& prod = prod_rule.production;
    const std::hash<std::string> str_hasher;
    std::size_t rule_hash = str_hasher(rule);

    std::size_t hash_mult = _HASH_MULTIPLIER;
    std::size_t prod_hash = 0x345678;
    std::size_t len = prod.size();
    for (const std::string& r : prod){
        prod_hash = (prod_hash ^ str_hasher(r)) * hash_mult;
        hash_mult += 82520 + len + len;
    }
    prod_hash += 97531;
    return prod_hash ^ rule_hash;
}

std::size_t parsing::ItemHasher::operator()(const lr_item_t& lr_item) const {
    ProdRuleHasher hasher;
    return hasher(lr_item.first) ^ static_cast<std::size_t>(lr_item.second);
}

/**
 * Borrowed from python3.6's frozenset hash
 */ 
static std::size_t _shuffle_bits(std::size_t h){
    return ((h ^ 89869747UL) ^ (h << 16)) * 3644798167UL;
}

std::size_t parsing::ItemSetHasher::operator()(const item_set_t& item_set) const {
    std::size_t hash = 0;
    ItemHasher item_hasher;
    for (const auto& lr_item : item_set){
        hash ^= _shuffle_bits(item_hasher(lr_item));  // entry hashes
    }
    hash ^= item_set.size() * 1927868237UL;  // # of active entrues
    hash = hash * 69069U + 907133923UL;
    return hash;
};
