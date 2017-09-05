#ifndef _PARSER_H
#define _PARSER_H

#include <vector>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <iostream>
#include <cassert>
#include <memory>

#include "utils.h"
#include "lexer.h"

namespace parsing {
    // Common nonterminals
    namespace nonterminals {
        const std::string EPSILON = "EMPTY";
    }

    // The function called for handling symbols in a production when reducing by a rule
    typedef std::shared_ptr<void> (*ParseCallback)(std::vector<std::shared_ptr<void>>& nodes, void* data);

    // Individual entries created by the user containing the 
    // - Nonterminal rule to be reduced to 
    // - The production of symbols that are reduced 
    // - The callback function handling the reduction
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

    // Items contained in the lr sets. These items contain 
    // - The parse rule itself
    // - The position in the production that the parser is in while parsing.
    typedef struct LRItem LRItem;
    struct LRItem {
        ParseRule parse_rule;
        std::size_t pos;

        bool operator==(const LRItem& other) const;
        std::string str() const;
    };

    struct LRItemHasher {
        std::size_t operator()(const LRItem& lr_item) const;
    };

    // The container holding the lr items. This is not exposed as a set for debugging
    // purposes to retain order of insertion into the set. We would like to print them 
    // in order. Internally, the vectors are converted to unordered sets when lookup
    // is necessary.
    typedef std::vector<LRItem> LRItemSet;

    struct LRItemSetHasher {
        std::size_t operator()(const LRItemSet&) const;
    };

    // Deterministic finite automata
    // This is essentially the set of item sets. Also implemented as a vector to 
    // retain insertion order for debugging in the grammar dump.
    typedef std::vector<LRItemSet> DFA;

    typedef struct ParseInstr ParseInstr;
    struct ParseInstr {
        enum Action {SHIFT, REDUCE, GOTO, ACCEPT} action;
        int value;

        bool operator==(const ParseInstr& other) const {
            return action == other.action && value == other.value;
        }
    };

    std::string action_str(const ParseInstr::Action&);

    enum Associativity {
        LEFT_ASSOC,
        RIGHT_ASSOC,
    };

    typedef std::vector<std::pair<enum Associativity, std::vector<std::string>>> PrecedenceList;
    typedef std::unordered_map<std::string, std::pair<std::size_t, enum Associativity>> PrecedenceTable;

    typedef struct ParserConflict ParserConflict;
    struct ParserConflict {
        std::size_t state;
        ParseInstr instr1;  // Default chosen instruction will be whatever appeared first in the rules
        ParseInstr instr2;
        std::string lookahead;
    };

    typedef std::unordered_map<std::size_t, std::unordered_map<std::string, ParseInstr>> ParseTable;

    // Extra functions for handling parse rule vectors
    std::vector<ParseRule> prepend_prime_rule(std::vector<ParseRule>);
    std::vector<ParseRule> trim_overload_tokens(std::vector<ParseRule>);

    void init_closure(LRItemSet&, const std::vector<ParseRule>&);
    LRItemSet move_pos(const LRItemSet&, const std::string&, const std::vector<ParseRule>&);
    DFA make_dfa(const std::vector<ParseRule>&);

    PrecedenceTable make_precedence_table(const PrecedenceList&);

    /**
     * Utility function for getting keys from a map.
     */
    template <typename T, typename U>
    std::unordered_set<T> keys(const std::unordered_map<T, U>& m){
        std::unordered_set<T> s;
        for (auto it = m.begin(); it != m.end(); ++it){
            s.insert(it->first);
        }
        return s;
    }


    /************** Grammar ************/ 

    /**
     * Class for storing all data structures used for the parser and debugging
     */
    class Grammar {
        private:
            const std::unordered_set<std::string> tokens_;

            // List of produciton rules 
            // This must be declared first immediately after the lexer b/c of constructor
            // member initialization order.
            const std::vector<ParseRule> parse_rules_with_overloads_;  
            const std::vector<ParseRule> parse_rules_;  

            // For Creating first/follow sets 
            const std::string start_nonterminal_;
            std::unordered_set<std::string> firsts_stack_;  // for keeping track of recursive calls 
            std::unordered_set<std::string> follows_stack_;
            std::unordered_map<std::string, std::unordered_set<std::string>> firsts_map_;
            std::unordered_map<std::string, std::unordered_set<std::string>> follows_map_;

            const PrecedenceTable precedence_map_;

            const DFA dfa_;

            ParseTable parse_table_;  // map of states to map of strings to parse instructions 

            std::vector<ParserConflict> conflicts_;

            // Methods
            bool is_terminal(const std::string&) const;
            std::string token_for_instr(const ParseInstr&, const std::string&) const;
            void update_with_precedence(const ParseInstr&, const ParseInstr&, const std::string&,
                                        std::size_t);
            std::string conflict_str(const ParseInstr&, const std::string lookahead = "") const;
            std::string rightmost_terminal(const std::vector<std::string>&) const;

            // For creating firsts/follows sets
            std::unordered_set<std::string> nonterminal_firsts(const std::string&);

        public:
            Grammar(const std::unordered_set<std::string>&, const std::vector<ParseRule>&,
                    const PrecedenceList& precedence={{}});

            void dump(std::ostream& stream=std::cerr) const;
            void dump_state(std::size_t, std::ostream& stream=std::cerr) const;
            
            // Firsts/follows methods 
            std::unordered_set<std::string> firsts(const std::string&);
            std::unordered_set<std::string> follows(const std::string&);

            // Getters
            const ParseTable& parse_table() const;
            const std::vector<ParseRule>& parse_rules() const;
            const std::vector<ParserConflict>& conflicts() const;
            const std::unordered_map<std::string, std::unordered_set<std::string>>& firsts() const;
            const std::unordered_map<std::string, std::unordered_set<std::string>>& follows() const;
    };


    /************** Parser ************/ 

    class Node;

    class NodeVisitor {
        public:
            virtual ~NodeVisitor(){}
            std::shared_ptr<void> visit(Node&);
    };

    /**
     * Usage:
     *
     * If you want to just create an AST for dumping into a string, the child node can just inherit 
     * from Node. 
     *
     * If you want to be able to visit nodes in the true, it just needs to inherit from 
     * Visitable which uses the CRTP to notify the Visitor to visit this specific node.
     *
     * The Node and Visitable classes are inherited virtually, so any other classes derived from Node 
     * can inherit virtually from Node to act as a mixin. 
     *
     * class BinExpr: public lang::SimpleNode, public lang::Visitable<BinExpr> {
     *     ...
     * };
     */ 
    class Node {
        public:
            virtual std::shared_ptr<void> accept(NodeVisitor&) = 0;
            virtual ~Node(){}

            // lines() returns a vector containing strings that represent 
            // individual lines separated in the code separated by newlines.
            virtual std::vector<std::string> lines() const = 0;

            // The lines joined by newlines
            std::string str() const { return join(lines(), "\n"); }
    };

    // Node that only contains one line
    class SimpleNode: public virtual Node {
        public:
            virtual std::string line() const = 0;
            std::vector<std::string> lines() const override { return {line()}; }
    };
    
    template <typename VisitingNode>
    class Visitor: public virtual NodeVisitor {
        public:
            virtual std::shared_ptr<void> visit(VisitingNode&) = 0;
    };

    template <typename DerivedNode>
    class Visitable: public virtual Node {
        public:
            std::shared_ptr<void> accept(NodeVisitor& base_visitor){
                try {
                    Visitor<DerivedNode>& visitor = dynamic_cast<Visitor<DerivedNode>&>(base_visitor);
                    return visitor.visit(static_cast<DerivedNode&>(*this));
                } catch (const std::bad_cast& e){
                    std::ostringstream err;
                    err << "Bad cast thrown for: " << typeid(DerivedNode).name() << std::endl;
                    err << "Check if your Visitor implementation inherits from both 'Visitor<NODE>' and implements 'std::shared_ptr<void> visit(NODE&)'." << std::endl;
                    throw std::runtime_error(err.str());
                }
            }
    };

    class Parser {
        private:
            lexing::Lexer& lexer_;
            const Grammar grammar_;

            void reduce(const ParseRule&, std::vector<lexing::LexToken>&, std::vector<std::shared_ptr<void>>&,
                        std::vector<std::size_t>&, void* data);
            const ParseInstr& get_instr(std::size_t, const lexing::LexToken&);

        public:
            Parser(lexing::Lexer&, const Grammar& table);
            Parser(lexing::Lexer&, const std::vector<ParseRule>& parse_rules,
                   const PrecedenceList& precedence={{}});

            std::shared_ptr<void> parse(const std::string&, void* data=nullptr);

            // Getters
            const Grammar& grammar() const;
    };

    // Runtime error on finding a parse instruction that does not exist 
    // for a state and lookahead.
    class ParseError: public std::runtime_error {
        private:
            const Parser& parser_;
            const std::size_t state_;
            const lexing::LexToken lookahead_;

        public:
            ParseError(const Parser&, const std::size_t, const lexing::LexToken&);
            virtual const char* what() const throw();
    };
}

#endif
