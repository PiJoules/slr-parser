#ifndef _PARSER_H
#define _PARSER_H

#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <cassert>

#include "lexer.h"

namespace parsing {
    // Common nonterminals
    namespace nonterminals {
        const std::string EPSILON = "EMPTY";
    }

    // The function called for handling symbols in a production when reducing by a rule
    typedef void* (*ParseCallback)(std::vector<void*>& nodes, void* data);

    typedef struct ParseRule ParseRule;
    struct ParseRule {
        std::string rule;
        std::vector<std::string> production;
        ParseCallback callback;
        bool operator==(const ParseRule& other) const;
    };

    struct ProdRuleHasher {
        std::size_t operator()(const ParseRule& prod_rule) const;
    };

    // Parse table generation
    typedef std::pair<ParseRule, int> lr_item_t;
    struct ItemHasher {
        std::size_t operator()(const lr_item_t& lr_item) const;
    };
    // TODO: Create an immutable container type (like tuples/frozensets in python)
    // so that we don't have to take the hash of a mutable container set at compile time.
    typedef std::unordered_set<lr_item_t, ItemHasher> item_set_t;

    struct ItemSetHasher {
        std::size_t operator()(const item_set_t&) const;
    };
    typedef std::unordered_set<item_set_t, ItemSetHasher> dfa_t;

    void init_closure(item_set_t&, const std::vector<ParseRule>&);
    item_set_t move_pos(const item_set_t&, const std::string&, const std::vector<ParseRule>&);
    void init_dfa(dfa_t& dfa, const std::vector<ParseRule>&);

    typedef struct ParseInstr ParseInstr;
    struct ParseInstr {
        enum Action {SHIFT, REDUCE, GOTO, ACCEPT} action;
        int value;
    };
    typedef std::unordered_map<int, std::unordered_map<std::string, ParseInstr>> parse_table_t;

    enum Associativity {
        LEFT_ASSOC,
        RIGHT_ASSOC,
        // Ply also implements nonassociativity, but ignoring that for now
    };
    typedef std::vector<std::pair<enum Associativity, std::vector<std::string>>> precedence_t;
    typedef struct {
        ParseInstr instr1;  // Default chosen instruction will be whatever appeared first in the rules
        ParseInstr instr2;
        std::string lookahead;
    } ParserConflict;


    /******** Nodes ***********/ 

    class Node {
        public:
            virtual std::vector<std::string> lines() const {
                std::vector<std::string> v;
                return v;
            };

            // The lines joined by newlines
            std::string str() const;

            virtual ~Node(){}
    };

    class LexTokenWrapper: public Node {
        private:
            lexing::LexToken token_;

        public:
            LexTokenWrapper(const lexing::LexToken&);
            LexTokenWrapper& operator=(const lexing::LexToken& other){
                token_ = other;
                return *this;
            }
            lexing::LexToken token() const;
            virtual std::vector<std::string> lines() const;
    };

    class Parser {
        private:
            lexing::Lexer& lexer_;

            // For Creating first/follow sets 
            std::unordered_set<std::string> nonterminals_;
            std::string start_nonterminal_;
            std::unordered_set<std::string> firsts_stack_;  // for keeping track of recursive calls 
            std::unordered_set<std::string> follows_stack_;
            std::unordered_map<std::string, std::unordered_set<std::string>> firsts_map_;  // memoization
            std::unordered_map<std::string, std::unordered_set<std::string>> follows_map_;

            item_set_t top_item_set_;
            std::vector<ParseRule> prod_rules_;  // list of produciton rules
            parse_table_t parse_table_;  // map of states to map of strings to parse instructions
            std::unordered_map<const item_set_t, int, ItemSetHasher> item_set_map_;  // map of item sets (states) to their state number
            std::unordered_map<const ParseRule, int, ProdRuleHasher> prod_rule_map_;  // map of production rule index to production rule (flipped keys + vals of prod_rules_)
            std::unordered_map<std::string, std::pair<std::size_t, enum Associativity>> precedence_map_;  // map of symbol to pair of the precedence value and associativity
            std::vector<ParserConflict> conflicts_;

            /******* Methods ********/
            void init_parse_table(const dfa_t&);
            bool is_terminal(const std::string&) const;
            void init_precedence(const precedence_t&);
            std::string key_for_instr(const ParseInstr&, const std::string&) const;
            void check_precedence(const ParseInstr&, const ParseInstr&, const std::string&,
                    std::unordered_map<std::string, ParseInstr>&);
            std::string conflict_str(const ParseInstr&, const std::string lookahead = "") const;
            std::string rightmost_terminal(const std::vector<std::string>&) const;
            const ParseInstr& get_instr(std::size_t, const lexing::LexToken&);

            // For creating firsts/follows sets
            std::unordered_set<std::string> make_nonterminal_firsts(const std::string&);

            void reduce(const ParseRule&, std::vector<lexing::LexToken>&, std::vector<void*>&,
                        std::vector<std::size_t>&, void* data);

        public:
            Parser(lexing::Lexer&, const std::vector<ParseRule>& prod_rules,
                   const precedence_t& precedence={{}});
            void dump_grammar(std::ostream& stream=std::cerr) const;
            void dump_state(std::size_t, std::ostream& stream=std::cerr) const;
            const std::vector<ParserConflict>& conflicts() const;
            void* parse(const std::string&, void* data=nullptr);
            
            // Firsts/follows methods 
            std::unordered_set<std::string> firsts(const std::string&);
            std::unordered_set<std::string> follows(const std::string&);
            const std::unordered_set<std::string>& firsts() const;
            const std::unordered_set<std::string>& follows() const;
            const std::unordered_set<std::string>& firsts_stack() const;
            const std::unordered_set<std::string>& follows_stack() const;
    };

    std::string str(const std::vector<std::string>& production);
    std::string str(const ParseRule& prod_rule);
    std::string str(const lr_item_t& lr_item);
    std::string str(const item_set_t& item_set);
    std::string str(const ParseInstr::Action&);
}

#endif
