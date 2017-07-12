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

void lexing::Lexer::advance(int count){
    pos_ += count;
    colno_ += count;
}

void lexing::Lexer::advancenl(int count){
    pos_ += count;
    lineno_ += count;
    colno_ = 1;
}

/**
 * Return the next token and advance the stream.
 */
lexing::LexToken lexing::Lexer::token(void* data){
    LexToken next_token;

    next_token.pos = pos_;
    next_token.lineno = lineno_;
    next_token.colno = colno_;

    // Nothing else
    if (empty()){
        next_token.symbol = tokens::END;
        next_token.value = "";
        return next_token;
    }

    std::string match;
    std::smatch matches;
    auto it = tokens_.cbegin();
    for (; it != tokens_.cend(); ++it){
        const std::string& symbol = it->first;
        const std::regex& re = it->second.first;
        const TokenCallback& callback = it->second.second;

        if (std::regex_search(lexcode_, matches, re, std::regex_constants::match_continuous)){
            // Found 
            match = matches[0];
            next_token.symbol = symbol;
            next_token.value = match;

            if (callback){
                callback(next_token, data);
            }

            break;
        }
    }

    // No matches
    if (it == tokens_.end()){
        if (isspace(lexcode_.front())){
            // Check if whitespace that was not caught as a token 
            // Trim whitespace, advance position, then try to load again 
            while (isspace(lexcode_.front())){
                if (lexcode_.front() == '\n'){
                    advancenl();
                }
                else {
                    advance();
                }
                lexcode_.erase(0, 1);
            }
            next_token = token(data);
        }
        else {
            // None of the regex's matched
            std::ostringstream err;
            err << "Unexpected character '" << lexcode_.front() << "' did not match the start of any tokens.";
            throw std::runtime_error(err.str());
        }
    }

    // Advance the stream 
    // Count newlines that may be in the match'd string
    for (const char c : match){
        if (c == '\n'){
            advancenl();
        }
        else {
            advance();
        }
    }

    // Remove this part of string and update position
    lexcode_.erase(0, match.size());
    
    return next_token;
}
