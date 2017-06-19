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

#include <iostream>
#include <cassert>

// Borrowed from python hash
#define _HASH_MULTIPLIER 1000003

#define NEWLINE_C '\n'
#define ADD_C '+'
#define SUB_C '-'
#define MUL_C '*'
#define DIV_C '/'
#define UNDERSCORE_C '_'

namespace lang {
    // All tokens will be positive or zero. All production rules will be negative.
    //
    // BE SURE TO ADD THE STRING REPRESENTATION OF EACH NEW TOKEN TO THE SWITCH
    // STMT IN LANG_UTILS.CPP FOR DEBUGGING.
    enum Symbol {
        // Values 
        int_tok=0,
        name_tok=1,
        
        // Binary operations 
        add_tok=50,
        sub_tok=51,
        mul_tok=52,
        div_tok=53,

        // Misc 
        newline_tok=200,
        whitespace_tok=201,
        indent_tok=202,
        dedent_tok=203,
        eof_tok=204,

        // Parser rules 
        module_rule=-1,
        funcdef_rule=-2,
        expr_rule=-50,
    };

    struct SymbolHasher {
        std::size_t operator()(const enum Symbol&) const;
    };

    bool is_token(const enum Symbol& symbol);
    bool is_rule(const enum Symbol& symbol);

    typedef struct LexToken LexToken;
    struct LexToken {
        enum Symbol symbol;
        std::string value;
        int pos, lineno, colno;

        const std::string str() const {
            std::ostringstream s;
            s << "{" << "symbol: " << symbol << ", value: '" << value 
              << "', pos: " << pos << ", lineno: " << lineno << ", colno: "
              << colno << "}";  
            return s.str();
        }
    };

    // Custom exceptions 
    class IndentationError: public std::runtime_error {
        private:
            int lineno_;

        public:
            IndentationError(int lineno): std::runtime_error("Indentation error"),
                lineno_(lineno){}
            virtual const char* what() const throw() {
                std::ostringstream err;
                err << std::runtime_error::what();
                err << "Unexpected indentation on line " << lineno_ << ".";
                return err.str().c_str();
            }
    };

    class Lexer {
        private:
            std::stringstream code_stream;
            int pos, lineno, colno;
            LexToken next_tok;
            void load_next_tok();

            // Indentation tracking
            std::vector<int> levels;
            bool found_indent, found_dedent;

            // Lexer rules
            LexToken scan_char(enum Symbol);
            LexToken scan_name();
            LexToken scan_int();
            LexToken scan_newline();

        public:
            Lexer(): pos(1), lineno(1), colno(1), levels({1}),
                found_indent(false), found_dedent(false){
                input("");  // Initialize
            };
            void input(const std::string& code);
            LexToken token();
            LexToken peek() const;
            bool eof();
    };

    class LangNode {};

    class Stmt: public LangNode {};
    class Value: public LangNode {};
    class ModuleStmt: public Stmt {};

    class Module: public LangNode {
        public:
            std::vector<ModuleStmt> body_;
            Module(const std::vector<ModuleStmt> body): body_(body){}
    };

    class Name: public Value {
        public:
            std::string id_;
            Name(const std::string id): id_(id){}
    };

    // Shift-reduce parsing
    typedef std::vector<enum Symbol> production_t;
    typedef std::pair<enum Symbol, production_t> prod_rule_t;

    struct ProdRuleHasher {
        std::size_t operator()(const prod_rule_t& prod_rule) const;
    };

    // Parse table generation
    typedef std::pair<prod_rule_t, int> lr_item_t;
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

    void init_closure(item_set_t&, const std::vector<prod_rule_t>&);
    item_set_t move_pos(const item_set_t&, const enum Symbol&, const std::vector<prod_rule_t>&);
    void init_dfa(dfa_t& dfa, const std::vector<prod_rule_t>&);

    typedef struct ParseInstr ParseInstr;
    struct ParseInstr {
        enum Action {SHIFT, REDUCE, GOTO, ACCEPT} action;
        int value;
    };
    typedef std::unordered_map<int, std::unordered_map<enum Symbol, ParseInstr, SymbolHasher>> parse_table_t;

    /******** Parser ********/ 

    extern const std::vector<prod_rule_t> LANG_RULES;

    class Parser {
        private:
            Lexer lexer;
            const std::vector<prod_rule_t>& prod_rules_;
            parse_table_t parse_table_;
            std::unordered_map<const item_set_t, int, ItemSetHasher> item_set_map_;
            std::unordered_map<const prod_rule_t, int, ProdRuleHasher> prod_rule_map_;

            void init_parse_table(const dfa_t&);

        public:
            Parser(const std::vector<prod_rule_t>& prod_rules);
            void input(const std::string&);
            void dump_grammar(std::ostream& stream=std::cout) const;
    };

    /**
     * Debugging
     */
    std::string str(const enum Symbol&);
    std::string str(const production_t& production);
    std::string str(const prod_rule_t& prod_rule);
    std::string str(const lr_item_t& lr_item);
}

#endif
