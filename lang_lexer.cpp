#include "lang.h"

lang::LangLexer::LangLexer(const lexing::TokensMap& tokens): lexing::Lexer(tokens){}

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
    assert(!(found_dedent_ && found_indent_));

    if (!loaded_init_token_){
        next_tok_ = lexing::Lexer::token();
        loaded_init_token_ = true;
    }

    if (found_indent_){
        found_indent_ = false;
        return make_indent();
    }
    else if (found_dedent_){
        found_dedent_ = false;
        return make_dedent();
    }
    else if (next_tok_.symbol == lexing::tokens::END && levels_.size() > 1){
        // If we are in a situation where we have reached the end of a file, but 
        // still have not fully dedent'd back to the start, keep returning dedents.
        levels_.pop_back();
        return make_dedent();
    }

    lexing::LexToken tok = next_tok_;
    next_tok_ = lexing::Lexer::token();

    if (tok.symbol == lang::tokens::NEWLINE){
        // A NEWLINE token represents a series of lines separated by whitespace 
        // which includes at least 1 \n:
        // Example:
        // x 
        //   
        //   y 
        //
        // Between x and y are newlines and spaces. The spaces are consumed as comments 
        // and the separate newlines are reprented as one NEWLINE by consuming all 
        // next_tokens that are also NEWLINEs.
        while (next_tok_.symbol == lang::tokens::NEWLINE){
            next_tok_ = lexing::Lexer::token();
        }

        int next_col = next_tok_.colno;

        // The next token to be returned may be an indent or dedent 
        int last_level = levels_.back();
        if (next_col > last_level){
            // Indent 
            // Return order: NEWLINE, INDENT, next_tok.symbol
            levels_.push_back(next_col);
            found_indent_ = true;
        }
        else if (next_col < last_level){
            // Dedent 
            // Return order: NEWLINE, DEDENT, next_tok.symbol
            levels_.pop_back();

            // Make sure the indentations match any of the previous ones 
            if (!std::any_of(levels_.begin(), levels_.end(), [next_col](int lvl){ return lvl == next_col; })){
                throw lang::IndentationError(next_tok_.lineno);
            }

            found_dedent_ = true;
        }
    }

    return tok;
}
