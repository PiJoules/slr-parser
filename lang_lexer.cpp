#include "lang.h"

lang::LangLexer::LangLexer(const lexing::TokensMap& tokens): lexing::Lexer(tokens){}

/**
 * Initialize next_tok_.
 */
void lang::LangLexer::load_next_tok(){
    next_tok_ = lexing::Lexer::token(nullptr);

    // If the token is a comment, ignore and try again
    if (next_tok_.symbol == lexing::tokens::COMMENT){
        load_next_tok();
    }
}

void lang::LangLexer::input(const std::string& code){
    lexing::Lexer::input(code);
    load_next_tok();
}

lexing::LexToken lang::LangLexer::make_indent() const {
    return {tokens::INDENT, "", pos(), lineno(), 1};
}

lexing::LexToken lang::LangLexer::make_dedent() const {
    return {tokens::DEDENT, "", pos(), lineno(), 1};
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
lexing::LexToken lang::LangLexer::token(){
    // Should not both be true at same time
    assert(!(found_dedent && found_indent));
    lexing::LexToken tok = next_tok_;

    if (found_indent){
        found_indent = false;
        return make_indent();
    }
    else if (found_dedent){
        found_dedent = false;
        return make_dedent();
    }

    load_next_tok();

    if (tok.symbol == lexing::tokens::NEWLINE){
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
                throw lang::IndentationError(next_tok_.lineno);
            }

            found_dedent = true;
        }
    }
    return tok;
}
