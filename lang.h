#ifndef _LANG_H
#define _LANG_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <stack>
#include <utility>
#include <algorithm>

#include <iostream>
#include <cassert>

#define NEWLINE_C '\n'
#define ADD_C '+'
#define SUB_C '-'
#define UNDERSCORE_C '_'

namespace lang {
    enum Symbol {
        // Values 
        int_tok=1,
        name_tok=2,
        
        // Binary operations 
        add_tok=50,
        sub_tok=51,

        // Misc 
        newline_tok=200,
        whitespace_tok=201,
        indent_tok=202,
        dedent_tok=203,
        eof_tok=204,

        // Parser rules 
        module_rule=-1,
        funcdef_rule=-2,
    };

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
    class ModuleStmt: public Stmt {};

    class Module: public LangNode {
        public:
            std::vector<ModuleStmt> body;
    };

    class Parser {
        private:
            Lexer lexer;

        public:
            void input(const std::string&);

            LangNode parse_module();
            std::vector<ModuleStmt> parse_module_stmt_list();
            LangNode parse_module_stmt();

            void accept_terminal(const std::string& terminal);

            bool check_terminal(const std::string& terminal) const;
            bool check_module_stmt();
    };
}

#endif
