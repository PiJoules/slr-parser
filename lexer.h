#ifndef _LEXER_H
#define _LEXER_H

#include <unordered_map>
#include <regex>
#include <stdexcept>
#include <sstream>

namespace lexing {
    // Common token names
    namespace tokens {
        const std::string NEWLINE = "NEWLINE";
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

    class Lexer {
        private:
            std::string lexcode_;
            int pos_ = 1, lineno_ = 1, colno_ = 1;
            const TokensMapRegex tokens_;

            TokensMapRegex to_regex_map(const TokensMap&) const;

        public:
            Lexer(const TokensMap&);
            Lexer(const TokensMapRegex&);

            void input(const std::string&);
            LexToken token(void* data=nullptr);
            bool empty() const;
            void advance(int count=1);
            void advancenl(int count=1);

            // Getters
            int pos() const;
            int lineno() const;
            int colno() const;
            const TokensMapRegex& tokens() const;
    };
}

#endif
