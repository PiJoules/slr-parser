#ifndef _LEXER_H
#define _LEXER_H

#include <stdexcept>

namespace lexing {
    namespace tokens {
        const std::string NEWLINE = "NEWLINE";
        const std::string END = "END";
        const std::string COMMENT = "COMMENT";
    }

    namespace nonterminals {
        const std::string EPSILON = "EMPTY";
    }

    struct LexToken {
        std::string symbol;
        std::string value;
        int pos, lineno, colno;
    };

    class Lexer;

    // Callback for handling a token found by the lexer.
    typedef void (*TokenCallback)(Lexer&, LexToken&);

    // A dict mapping a token to its regex and callback.
    typedef std::unordered_map<std::string, std::pair<std::string, TokenCallback>> TokensMap;

    class Lexer {
        private:
            std::string lexcode_;
            int pos_ = 1, lineno_ = 1, colno_ = 1;
            std::unordered_map<std::string, std::pair<std::regex, tok_callback_t>> tokens_;

            // Indentation tracking
            std::vector<int> levels = {1};
            bool found_indent = false, found_dedent = false;
            LexToken next_tok_ = {tokens::END, "", pos_, lineno_, colno_};
            void load_next_tok();
            LexToken make_indent() const;
            LexToken make_dedent() const;

        public:
            Lexer(const std::unordered_map<std::string, std::pair<std::string, TokenCallback>>&);
            void input(const std::string& code);
            LexToken token();
            const std::unordered_map<std::string, std::pair<std::regex, tok_callback_t>>& tokens() const;
            bool empty() const;
            void advance(int count=1);
            void advancenl(int count=1);
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
}

#endif
