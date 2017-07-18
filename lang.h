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

#include "lexer.h"
#include "parser.h"
#include "nodes.h"

namespace lang {

    /********* Lexer ********/
    namespace tokens {
        const std::string NEWLINE = "NEWLINE";
        const std::string INDENT = "INDENT";
        const std::string DEDENT = "DEDENT";
    };

    class LangLexer: public lexing::Lexer {
        private:
            lexing::LexToken make_indent() const;
            lexing::LexToken make_dedent() const;

            // Indentation tracking
            std::vector<int> levels = {1};
            bool found_indent = false, found_dedent = false;
            bool loaded_init_token_ = false;
            lexing::LexToken next_tok_;

        public:
            LangLexer(const lexing::TokensMap&);

            lexing::LexToken token(void* data) override;
    };

    // Custom exceptions 
    class IndentationError: public std::runtime_error {
        private:
            int lineno_;

        public:
            IndentationError(int lineno);
            virtual const char* what() const throw();
    };

    /******** Parser ********/ 

    extern const std::vector<parsing::ParseRule> LANG_RULES;
    extern const lexing::TokensMap LANG_TOKENS;
    extern const parsing::PrecedenceList LANG_PRECEDENCE;
}

#endif
