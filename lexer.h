#ifndef _LEXER_H
#define _LEXER_H

#include <unordered_map>
#include <regex>
#include <stdexcept>
#include <sstream>

namespace lexing {
    // Common token names
    namespace tokens {
        const std::string END = "END";  // End of input/file
        const std::string COMMENT = "COMMENT";
    }

    struct LexToken {
        std::string symbol;
        std::string value;
        int pos, lineno, colno;
        std::string str() const;
    };

    // Callback for handling a token found by the lexer.
    typedef void (*TokenCallback)(LexToken& token, void* data);

    // A dict mapping a token to its regex and callback.
    typedef std::unordered_map<std::string, std::pair<std::regex, TokenCallback>> TokensMapRegex;

    // Same as the above, but maps the token to the string representation of its regex
    typedef std::unordered_map<std::string, std::pair<std::string, TokenCallback>> TokensMap;

    // Simple conversion between the token maps.
    TokensMapRegex to_regex_map(const TokensMap&);

    class Lexer {
        private:
            std::string lexcode_;
            int pos_ = 1, lineno_ = 1, colno_ = 1;
            const TokensMapRegex tokens_;

            void advance_pos(char);
            void advance_stream_and_pos(const std::string&);
            void advance_stream_and_pos(char);
            bool find_match(LexToken&, void* data);

        public:
            Lexer(const TokensMap&);
            Lexer(const TokensMapRegex&);

            void input(const std::string&);
            virtual LexToken token(void* data);
            bool empty() const;

            // Getters
            int pos() const;
            int lineno() const;
            int colno() const;
            const TokensMapRegex& tokens() const;
            const std::string& lexcode() const;
    };

    // Runtime error on finding a start of string that does not match 
    // any regexs provided.
    class LexError: public std::runtime_error {
        private:
            const Lexer& lexer_;

        public:
            LexError(const Lexer&);
            virtual const char* what() const throw();
    };
}

#endif
