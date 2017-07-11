#include "lang.h"

#define NEWLINE_C '\n'

/**
 * Feed a string into the code stream.
 */
void lang::Lexer::input(const std::string& code){
    lexcode_ += code;
    load_next_tok();
}

/**
 * Checks if the stream has reached the end.
 */
bool lang::Lexer::empty() const {
    return lexcode_.empty();
}

/**
 * Constructor from map of string to regex string.
 */
lang::Lexer::Lexer(const tokens_map_t& tokens){
    tokens_.reserve(tokens.size());
    for (auto it = tokens.cbegin(); it != tokens.cend(); ++it){
        std::string key = it->first;
        auto value = it->second;
        auto existing = tokens_.find(key);
        if (existing == tokens_.end()){
            std::regex r(value.first);
            tokens_[key] = {r, value.second};
        }
        else {
            // Accidentally defined the token twice
            std::ostringstream err;
            err << "Token '" << key << "' already defined. Attempting to define again as '" << value.first << "'.";
            throw std::runtime_error(err.str());
        }
    }
}

void lang::Lexer::advance(int count){
    pos_ += count;
    colno_ += count;
}

void lang::Lexer::advancenl(int count){
    pos_ += count;
    lineno_ += count;
    colno_ = 1;
}

/**
 * Initialize next_tok_.
 */
void lang::Lexer::load_next_tok(){
    next_tok_.pos = pos_;
    next_tok_.lineno = lineno_;
    next_tok_.colno = colno_;

    // Nothing else
    if (empty()){
        next_tok_.symbol = tokens::END;
        next_tok_.value = "";
        return;
    }

    std::string match;
    std::smatch matches;
    auto it = tokens_.cbegin();
    for (; it != tokens_.cend(); ++it){
        std::string symbol = it->first;
        std::regex re = it->second.first;
        tok_callback_t callback = it->second.second;
        if (std::regex_search(lexcode_, matches, re, std::regex_constants::match_continuous)){
            // Found 
            match = matches[0];
            next_tok_.symbol = symbol;
            next_tok_.value = match;

            if (callback){
                next_tok_ = callback(this, next_tok_);
            }

            break;
        }
    }

    // No matches
    if (it == tokens_.cend()){
        if (isspace(lexcode_.front())){
            // Check if whitespace that was not caught as a token 
            // Trim whitespace, advance position, then try to load again 
            while (isspace(lexcode_.front())){
                if (lexcode_.front() == NEWLINE_C){
                    advancenl();
                }
                else {
                    advance();
                }
                lexcode_.erase(0, 1);
            }
            load_next_tok();
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
        if (c == NEWLINE_C){
            advancenl();
        }
        else {
            advance();
        }
    }

    // Remove this part of string and update position
    lexcode_.erase(0, match.size());

    // If the token is a comment, ignore and try again
    if (next_tok_.symbol == tokens::COMMENT){
        load_next_tok();
    }
}

lang::LexToken lang::Lexer::make_indent() const {
    return {tokens::INDENT, "", pos_, lineno_, 1};
}

lang::LexToken lang::Lexer::make_dedent() const {
    return {tokens::DEDENT, "", pos_, lineno_, 1};
}

/**
 * Keep track of indentation by tracking column numbers.
 *
 * Return an indent if the next token has a colno greater than the 
 * current token. After the indent, the next token is returned.
 *
 * Return a dedent if the next token has a colno less than the current.
 * After the dedent, the next token is returned.
 *
 * Otherwise, return the next token.
 */
lang::LexToken lang::Lexer::token(){
    // Should not both be true at same time
    assert(!(found_dedent && found_indent));
    LexToken tok = next_tok_;

    if (found_indent){
        found_indent = false;
        return make_indent();
    }
    else if (found_dedent){
        found_dedent = false;
        return make_dedent();
    }

    load_next_tok();

    if (tok.symbol == tokens::NEWLINE){
        int next_col = next_tok_.colno;

        // The next token to be returned may be an indent or dedent 
        int last_level = levels.back();
        if (next_col > last_level){
            // Indent 
            // Return order: NEWLINE, INDENT, next_tok.symbol
            levels.push_back(next_col);
            found_indent = true;
        }
        else if (next_col < last_level){
            // Dedent 
            // Return order: NEWLINE, DEDENT, next_tok.symbol
            levels.pop_back();

            // Make sure the indentations match any of the previous ones 
            if (!std::any_of(levels.begin(), levels.end(), [next_col](int lvl){ return lvl == next_col; })){
                throw IndentationError(next_tok_.lineno);
            }

            found_dedent = true;
        }
    }
    return tok;
}

const std::unordered_map<std::string, std::pair<std::regex, lang::tok_callback_t>>& lang::Lexer::tokens() const {
    return tokens_;
}
