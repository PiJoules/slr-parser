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
        std::string str() const;
    };

    struct ParseRuleHasher {
        std::size_t operator()(const ParseRule& parse_rule) const;
    };

    typedef struct LRItem LRItem;
    struct LRItem {
        ParseRule parse_rule;
        std::size_t pos;

        bool operator==(const LRItem& other) const;
        std::string str() const;
    };

    // Parse table generation
    struct ItemHasher {
        std::size_t operator()(const LRItem& lr_item) const;
    };
    // TODO: Create an immutable container type (like tuples/frozensets in python)
    // so that we don't have to take the hash of a mutable container set at compile time.
    typedef std::unordered_set<LRItem, ItemHasher> ItemSet;

    struct ItemSetHasher {
        std::size_t operator()(const ItemSet&) const;
    };

    // Deterministic finite automata
    typedef std::unordered_set<ItemSet, ItemSetHasher> DFA;

    typedef struct ParseInstr ParseInstr;
    struct ParseInstr {
        enum Action {SHIFT, REDUCE, GOTO, ACCEPT} action;
        int value;
    };

    std::string action_str(const ParseInstr::Action&);

    enum Associativity {
        LEFT_ASSOC,
        RIGHT_ASSOC,
        // TODO: Ply also implements nonassociativity, but ignoring that for now
    };

    typedef std::vector<std::pair<enum Associativity, std::vector<std::string>>> PrecedenceList;
    typedef std::unordered_map<std::string, std::pair<std::size_t, enum Associativity>> PrecedenceTable;

    typedef struct {
        ParseInstr instr1;  // Default chosen instruction will be whatever appeared first in the rules
        ParseInstr instr2;
        std::string lookahead;
    } ParserConflict;

    typedef std::unordered_map<std::size_t, std::unordered_map<std::string, ParseInstr>> ParseTable;

    void init_closure(ItemSet&, const std::vector<ParseRule>&);
    ItemSet move_pos(const ItemSet&, const std::string&, const std::vector<ParseRule>&);
    void init_dfa(DFA& dfa, const std::vector<ParseRule>&);


    /************** Parser ************/ 

    /**
     * Class for storing all data structures used for the parser and debugging
     */
    class Grammar {
        private:
            lexing::Lexer& lexer_;

            // For Creating first/follow sets 
            std::string start_nonterminal_;
            std::unordered_set<std::string> firsts_stack_;  // for keeping track of recursive calls 
            std::unordered_set<std::string> follows_stack_;
            std::unordered_map<std::string, std::unordered_set<std::string>> firsts_map_;  // memoization
            std::unordered_map<std::string, std::unordered_set<std::string>> follows_map_;

            ItemSet top_item_set_;
            PrecedenceTable precedence_map_;
            std::vector<ParseRule> parse_rules_;  // list of produciton rules
            ParseTable parse_table_;  // map of states to map of strings to parse instructions
            std::unordered_map<const ItemSet, int, ItemSetHasher> item_set_map_;  // map of item sets (states) to their state number
            std::unordered_map<const ParseRule, int, ParseRuleHasher> parse_rule_map_;  // map of production rule index to production rule (flipped keys + vals of parse_rules_)
            std::vector<ParserConflict> conflicts_;

            /******* Methods ********/
            void init_parse_table(const DFA&);
            bool is_terminal(const std::string&) const;
            void init_precedence(const PrecedenceList&);
            std::string key_for_instr(const ParseInstr&, const std::string&) const;
            void check_precedence(const ParseInstr&, const ParseInstr&, const std::string&,
                    std::unordered_map<std::string, ParseInstr>&);
            std::string conflict_str(const ParseInstr&, const std::string lookahead = "") const;
            std::string rightmost_terminal(const std::vector<std::string>&) const;

            // For creating firsts/follows sets
            std::unordered_set<std::string> make_nonterminal_firsts(const std::string&);

        public:
            Grammar(lexing::Lexer&, const std::vector<ParseRule>&,
                    const PrecedenceList& precedence={{}});

            void dump_grammar(std::ostream& stream=std::cerr) const;
            void dump_state(std::size_t, std::ostream& stream=std::cerr) const;
            const std::vector<ParserConflict>& conflicts() const;
            
            // Firsts/follows methods 
            std::unordered_set<std::string> firsts(const std::string&);
            std::unordered_set<std::string> follows(const std::string&);
            const std::unordered_set<std::string>& firsts() const;
            const std::unordered_set<std::string>& follows() const;
            const std::unordered_set<std::string>& firsts_stack() const;
            const std::unordered_set<std::string>& follows_stack() const;

            const ParseTable& parse_table() const;
            std::size_t init_state() const;
            const std::vector<ParseRule>& parse_rules() const;
    };

    Grammar make_grammar(lexing::Lexer&, const std::vector<ParseRule>&,
                         const PrecedenceList& precedence={{}});

    class Parser {
        private:
            lexing::Lexer& lexer_;
            const Grammar grammar_;

            void reduce(const ParseRule&, std::vector<lexing::LexToken>&, std::vector<void*>&,
                        std::vector<std::size_t>&, void* data);
            const ParseInstr& get_instr(std::size_t, const lexing::LexToken&);

        public:
            Parser(lexing::Lexer&, const Grammar& table);
            Parser(lexing::Lexer&, const std::vector<ParseRule>& parse_rules,
                   const PrecedenceList& precedence={{}});

            void* parse(const std::string&, void* data=nullptr);

            // Getters
            const Grammar& grammar() const;
    };


    /******** Base Nodes ***********/ 

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
}

#endif
