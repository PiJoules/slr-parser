#include "lang.h"

/**
 * Feed a string into the code stream.
 */
void lang::Lexer::input(const std::string& code){
    if (eof()){
        code_stream.clear();
    }
    code_stream << std::string(code);
    load_next_tok();
}

/**
 * Checks if the stream has reached the end.
 */
bool lang::Lexer::eof(){
    if (!code_stream){
        return true;
    }
    if (code_stream.eof()){
        return true;
    }
    return code_stream.peek() == EOF;
}

/**
 * Scan a single character from the stream as a token.
 */
lang::LexToken lang::Lexer::scan_char(enum Symbol base){
    LexToken tok = {
        base,
        std::string(1, code_stream.get()), 
        pos, 
        lineno, 
        colno
    };
    pos++;
    colno++;
    return tok;
}

static bool valid_name_char(char c){
    return isalnum(c) || c == UNDERSCORE_C;
}

/**
 * name ::= [a-zA-Z_] [a-zA-Z_]*
 */
lang::LexToken lang::Lexer::scan_name(){
    std::string name;
    LexToken tok;
    tok.pos = pos;
    tok.colno = colno;
    tok.lineno = lineno;
    tok.symbol = name_tok;
    do {
        name += code_stream.get();
        pos++;
        colno++;
    } while (valid_name_char(code_stream.peek()));
    tok.value = name;
    return tok;
}

/**
 * int ::= \d+
 */
lang::LexToken lang::Lexer::scan_int(){
    std::string num;
    LexToken tok;
    tok.pos = pos;
    tok.colno = colno;
    tok.lineno = lineno;
    tok.symbol = int_tok;
    do {
        num += code_stream.get();
        pos++;
        colno++;
    } while (isdigit(code_stream.peek()));
    tok.value = num;
    return tok;
}

/**
 * newline = \n+
 */ 
lang::LexToken lang::Lexer::scan_newline(){
    std::string newlines;
    LexToken tok;
    tok.pos = pos;
    tok.colno = colno;
    tok.lineno = lineno;
    tok.symbol = newline_tok;
    colno = 1;

    // Exhaust any more newlines 
    do {
        newlines += code_stream.get();
        pos++;
        lineno++;
    } while (code_stream.peek() == NEWLINE_C);
    tok.value = newlines;

    return tok;
}

/**
 * Scan a token from the stream.
 */
void lang::Lexer::load_next_tok(){
    while (!eof()){
        char lookahead = code_stream.peek();
        switch (lookahead){
            case ADD_C:
                next_tok = scan_char(add_tok);
                return;
            case SUB_C:
                next_tok = scan_char(sub_tok);
                return;
            case MUL_C:
                next_tok = scan_char(mul_tok);
                return;
            case DIV_C:
                next_tok = scan_char(div_tok);
                return;
            case UNDERSCORE_C:
                next_tok = scan_name();
                return;
            case NEWLINE_C:
                next_tok = scan_newline();
                return;
            default:
                if (isalpha(lookahead)){
                    next_tok = scan_name();
                    return;
                }
                else if (isdigit(lookahead)){
                    next_tok = scan_int();
                    return;
                }
                else if (isspace(lookahead) && lookahead != NEWLINE_C){
                    code_stream.get();

                    // Ignore all whitespace except newlines
                    pos++;
                    colno++;
                }
                else {
                    std::ostringstream err;
                    err << "Unknown character '" << lookahead << "' (" << static_cast<int>(lookahead) << ") at " << lineno << "," << colno + 1 << std::endl;
                    throw std::runtime_error(err.str());
                }
        }
    }

    next_tok = {
        eof_tok,
        "",
        pos,
        lineno,
        colno
    };
}

static lang::LexToken make_indent(int pos, int lineno, int colno){
    return {
        lang::indent_tok,
        "",
        pos,
        lineno,
        1
    };
}

static lang::LexToken make_dedent(int pos, int lineno, int colno){
    return {
        lang::dedent_tok,
        "",
        pos,
        lineno,
        1
    };
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
    LexToken tok = next_tok;

    if (found_indent){
        found_indent = false;
        return make_indent(pos, lineno, colno);
    }
    else if (found_dedent){
        found_dedent = false;
        return make_dedent(pos, lineno, colno);
    }

    load_next_tok();

    if (tok.symbol == newline_tok){
        int next_col = next_tok.colno;

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
                throw IndentationError(next_tok.lineno);
            }

            found_dedent = true;
        }
    }
    return tok;
}

lang::LexToken lang::Lexer::peek() const {
    return next_tok;
}
