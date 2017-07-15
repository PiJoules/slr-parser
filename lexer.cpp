#include "lexer.h"

/**
 * Getters
 */ 
int lexing::Lexer::pos() const { return pos_; }
int lexing::Lexer::lineno() const { return lineno_; }
int lexing::Lexer::colno() const { return colno_; }
const lexing::TokensMapRegex& lexing::Lexer::tokens() const { return tokens_; }

/**
 * Convert a TokensMap to a TokensMapRegex
 */
lexing::TokensMapRegex lexing::Lexer::to_regex_map(const TokensMap& token_map) const {
    TokensMapRegex regex_map;
    regex_map.reserve(token_map.size());

    for (auto it = token_map.begin(); it != token_map.end(); ++it){
        const std::string& token = it->first;
        const std::string& regex_str = it->second.first;
        const TokenCallback& callback = it->second.second;
        
        std::regex reg(regex_str);
        regex_map[token] = {reg, callback};
    }

    return regex_map;
}

lexing::Lexer::Lexer(const TokensMapRegex& tokens): tokens_(tokens){}
lexing::Lexer::Lexer(const TokensMap& tokens): tokens_(to_regex_map(tokens)){}

/**
 * Feed a string into the code stream.
 */
void lexing::Lexer::input(const std::string& code){
    lexcode_ += code;
}

/**
 * Checks if the stream has reached the end.
 */
bool lexing::Lexer::empty() const {
    return lexcode_.empty();
}

void lexing::Lexer::advance_pos(char c){
    pos_++;
    if (c == '\n'){
        lineno_++;
        colno_ = 1;
    }
    else {
        colno_++;
    }

}

void lexing::Lexer::advance_stream_and_pos(const std::string& s){
    // Advance the position
    for (const char c : s){
        advance_pos(c);
    }

    // Remove this part of string
    lexcode_.erase(0, s.size());
}

void lexing::Lexer::advance_stream_and_pos(char c){
    advance_pos(c);
    lexcode_.erase(0, 1);
}

bool lexing::Lexer::find_match(LexToken& next_token, void* data){
    next_token.pos = pos_;
    next_token.lineno = lineno_;
    next_token.colno = colno_;

    // Nothing else
    if (empty()){
        next_token.symbol = tokens::END;
        next_token.value = "";
        return true;
    }

    std::smatch matches;
    auto it = tokens_.cbegin();
    for (; it != tokens_.cend(); ++it){
        const std::string& symbol = it->first;
        const std::regex& re = it->second.first;
        const TokenCallback& callback = it->second.second;

        if (std::regex_search(lexcode_, matches, re, std::regex_constants::match_continuous)){
            // Found 
            const std::string& match = matches[0];
            next_token.symbol = symbol;
            next_token.value = match;

            // Found, so advance the stream 
            // Count newlines that may be in the match'd string
            advance_stream_and_pos(match);

            // Then run the callback after processing
            if (callback){
                callback(next_token, data);
            }

            return true;
        }
    }
    return false;
}

/**
 * Return the next token and advance the stream.
 */
lexing::LexToken lexing::Lexer::token(void* data){
    LexToken next_token;

    do {
        bool found = find_match(next_token, data);
        while (!found){
            // No matches
            if (isspace(lexcode_.front())){
                // Check if whitespace that was not caught as a token 
                // Trim whitespace, advance position, then try to load again 
                char c = lexcode_.front();
                while (isspace(c)){
                    advance_stream_and_pos(c);
                    c = lexcode_.front();
                }
            }
            else {
                // None of the regex's matched
                std::ostringstream err;
                err << "Unexpected character '" << lexcode_.front() << "' did not match the start of any tokens.";
                throw std::runtime_error(err.str());
            }

            // Try again
            found = find_match(next_token, data);
        }

    // Ignore comments
    } while (next_token.symbol == lexing::tokens::COMMENT);

    return next_token;
}

std::string lexing::LexToken::str() const {
    std::ostringstream s;
    s << "{" << "symbol: " << symbol << ", value: '" << value 
      << "', pos: " << pos << ", lineno: " << lineno << ", colno: "
      << colno << "}";  
    return s.str();
}
