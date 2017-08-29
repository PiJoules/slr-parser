#include "lexer.h"

/********** LexToken *********/

/**
 * String representation of a lextoken for debugging.
 */
std::string lexing::LexToken::str() const {
    std::ostringstream s;
    s << "{" << "symbol: " << symbol << ", value: '" << value 
      << "', pos: " << pos << ", lineno: " << lineno << ", colno: "
      << colno << "}";  
    return s.str();
}

/**
 * Convert a TokensMap to a TokensMapRegex
 */
lexing::TokensMapRegex lexing::to_regex_map(const TokensMap& token_map){
    TokensMapRegex regex_map;
    regex_map.reserve(token_map.size());

    for (auto it = token_map.begin(); it != token_map.end(); ++it){
        const std::string& token = it->first;
        const std::string& regex_str = it->second.first;
        const TokenCallback& callback = it->second.second;
        
        if (!regex_str.empty()){
            std::regex reg(regex_str);
            regex_map[token] = {reg, callback};
        }
    }

    return regex_map;
}

/********* Lexer ************/

/**
 * Advance the position, and column number or line number depending on the character.
 */
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

/**
 * Advance the position and trim the start of the saved code.
 */
void lexing::Lexer::advance_stream_and_pos(const std::string& s){
    // Advance the position
    for (const char c : s){
        advance_pos(c);
    }

    // Remove this part of string
    lexcode_.erase(0, s.size());
}

/**
 * Same as advance_stream_and_pos but for a single character.
 */
void lexing::Lexer::advance_stream_and_pos(char c){
    advance_pos(c);
    lexcode_.erase(0, 1);
}

/** 
 * Search the tokens map for a regex that matches the start of the stream.
 *
 * @param next_token The token that will contain the matched value for the regex found.
 * @param data Miscellanious data that the user implements and may edit on matching specific tokens.
 *
 * @return true if any of the regexs provided match the start of the lexcode_.
 */
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
    for (auto it = tokens_.begin(); it != tokens_.end(); ++it){
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
 * Constructors
 */ 
lexing::Lexer::Lexer(const TokensMap& tokens): tokens_map_(tokens), tokens_(to_regex_map(tokens)){}

/**
 * Feed a string into the code stream.
 */
void lexing::Lexer::input(const std::string& code){
    lexcode_ += code;
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
                throw LexError(*this);
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

/**
 * Checks if the stream has reached the end.
 */
bool lexing::Lexer::empty() const {
    return lexcode_.empty();
}

/**
 * Getters
 */ 
int lexing::Lexer::pos() const { return pos_; }
int lexing::Lexer::lineno() const { return lineno_; }
int lexing::Lexer::colno() const { return colno_; }
const lexing::TokensMap& lexing::Lexer::tokens() const { return tokens_map_; }
const std::string& lexing::Lexer::lexcode() const { return lexcode_; }

/************* LexError ************/ 

lexing::LexError::LexError(const Lexer& lexer): std::runtime_error("Lexer error"), lexer_(lexer){}

const char* lexing::LexError::what() const throw(){
    std::ostringstream err;
    err << std::runtime_error::what();
    err << ": Start of string '" << lexer_.lexcode().substr(0, 10) << "' did not match the start of any tokens. Line " << lexer_.lineno() << ", col " << lexer_.colno() << ".";
    return err.str().c_str();
}
