#ifndef _LANG_H
#define _LANG_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <ios>

#define NEWLINE_C '\n'
#define ADD_C '+'
#define SUB_C '-'
#define UNDERSCORE_C '_'

namespace lang {

    enum Token {
        // Values
        int_tok,
        name_tok,

        // Binary expressions
        add_tok,
        sub_tok,
    };

    typedef struct LexToken LexToken;
    struct LexToken {
        std::string value;
        int pos, lineno, colno;
    };


    class Lexer {
        private:
            std::stringstream code_stream;
            int pos = 1, lineno = 1, colno = 1;

            // Lexer rules
            LexToken scan_char();
            LexToken scan_name();
            LexToken scan_int();

        public:
            //Lexer(): code_stream(std::ios_base::in | std::ios_base::out | std::ios_base::ate){}
            void input(const std::string& code);
            LexToken token();
            bool eof();
    };

}

#endif
