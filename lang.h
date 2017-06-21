#ifndef _LANG_H
#define _LANG_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <stack>
#include <utility>
#include <tuple>
#include <algorithm>
#include <unordered_set>
#include <regex>

#include <iostream>
#include <cassert>

// Borrowed from python hash
#define _HASH_MULTIPLIER 1000003

namespace lang {
    namespace tokens {
        const std::string NEWLINE = "\n";
        const std::string INDENT = "INDENT";
        const std::string DEDENT = "DEDENT";
        const std::string END = "END";
    }

    typedef struct LexToken LexToken;
    struct LexToken {
        std::string symbol;
        std::string value;
        int pos, lineno, colno;
    };

    // Custom exceptions 
    class IndentationError: public std::runtime_error {
        private:
            int lineno_;

        public:
            IndentationError(int lineno): std::runtime_error("Indentation error"),
                lineno_(lineno){}
            virtual const char* what() const throw();
    };

    class Lexer {
        private:
            std::string lexcode_;
            int pos_ = 1, lineno_ = 1, colno_ = 1;
            std::unordered_map<std::string, std::regex> tokens_;

            // Indentation tracking
            std::vector<int> levels = {1};
            bool found_indent = false, found_dedent = false;
            LexToken next_tok_ = {tokens::END, "", pos_, lineno_, colno_};
            void load_next_tok();
            LexToken make_indent() const;
            LexToken make_dedent() const;

        public:
            Lexer(const std::unordered_map<std::string, std::string>&);
            void input(const std::string& code);
            LexToken token();
            bool empty() const;
            void advance(int count=1);
            void advancenl(int count=1);
    };

    //class LangNode {};

    //class Stmt: public LangNode {};
    //class Value: public LangNode {};
    //class ModuleStmt: public Stmt {};

    //class Module: public LangNode {
    //    public:
    //        std::vector<ModuleStmt> body_;
    //        Module(const std::vector<ModuleStmt> body): body_(body){}
    //};

    //class Name: public Value {
    //    public:
    //        std::string id_;
    //        Name(const std::string id): id_(id){}
    //};

    //// Shift-reduce parsing
    //typedef std::vector<enum Symbol> production_t;
    //typedef std::pair<enum Symbol, production_t> prod_rule_t;

    //struct ProdRuleHasher {
    //    std::size_t operator()(const prod_rule_t& prod_rule) const;
    //};

    //// Parse table generation
    //typedef std::pair<prod_rule_t, int> lr_item_t;
    //struct ItemHasher {
    //    std::size_t operator()(const lr_item_t& lr_item) const;
    //};
    //// TODO: Create an immutable container type (like tuples/frozensets in python)
    //// so that we don't have to take the hash of a mutable container set at compile time.
    //typedef std::unordered_set<lr_item_t, ItemHasher> item_set_t;

    //struct ItemSetHasher {
    //    std::size_t operator()(const item_set_t&) const;
    //};
    //typedef std::unordered_set<item_set_t, ItemSetHasher> dfa_t;

    //void init_closure(item_set_t&, const std::vector<prod_rule_t>&);
    //item_set_t move_pos(const item_set_t&, const enum Symbol&, const std::vector<prod_rule_t>&);
    //void init_dfa(dfa_t& dfa, const std::vector<prod_rule_t>&);

    //typedef struct ParseInstr ParseInstr;
    //struct ParseInstr {
    //    enum Action {SHIFT, REDUCE, GOTO, ACCEPT} action;
    //    int value;
    //};
    //typedef std::unordered_map<int, std::unordered_map<enum Symbol, ParseInstr, SymbolHasher>> parse_table_t;

    ///******** Parser ********/ 

    //extern const std::vector<prod_rule_t> LANG_RULES;

    //class Parser {
    //    private:
    //        Lexer lexer;
    //        const std::vector<prod_rule_t>& prod_rules_;
    //        parse_table_t parse_table_;
    //        std::unordered_map<const item_set_t, int, ItemSetHasher> item_set_map_;
    //        std::unordered_map<const prod_rule_t, int, ProdRuleHasher> prod_rule_map_;

    //        void init_parse_table(const dfa_t&);

    //    public:
    //        Parser(const std::vector<prod_rule_t>& prod_rules);
    //        void input(const std::string&);
    //        void dump_grammar(std::ostream& stream=std::cout) const;
    //};

    /**
     * Debugging
     */ 
    std::string str(const LexToken&);
    //std::string str(const enum Symbol&);
    //std::string str(const production_t& production);
    //std::string str(const prod_rule_t& prod_rule);
    //std::string str(const lr_item_t& lr_item);
}

#endif
