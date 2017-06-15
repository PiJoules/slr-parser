#include "lang.h"
#include <cassert>

/**
 * Feed a string into the code stream.
 */
void lang::Lexer::input(const std::string& code){
    if (eof()){
        code_stream.clear();
    }
    code_stream << std::string(code);
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
 * Scan a single character from the stream.
 */
lang::LexToken lang::Lexer::scan_char(){
    LexToken tok = {std::string(1, code_stream.get()), pos, lineno, colno};
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
    do {
        num += code_stream.get();
        pos++;
        colno++;
    } while (isdigit(code_stream.peek()));
    tok.value = num;
    return tok;
}

/**
 * Scan a token from the stream.
 */
lang::LexToken lang::Lexer::token(){
    while (!eof()){
        char lookahead = code_stream.peek();
        switch (lookahead){
            case ADD_C:
            case SUB_C:
                return scan_char();
            case UNDERSCORE_C:
                return scan_name();
            default:
                if (isalpha(lookahead)){
                    return scan_name();
                }
                else if (isdigit(lookahead)){
                    return scan_int();
                }
                else if (isspace(lookahead)){
                    pos++;
                    if (lookahead == NEWLINE_C){
                        lineno++;
                        colno = 1;
                    }
                    else {
                        colno++;
                    }

                    // Ignore whitespace
                    code_stream.get();
                }
                else {
                    std::ostringstream err;
                    err << "Unknown character '" << lookahead << "' (" << static_cast<int>(lookahead) << ") at " << lineno << "," << colno + 1 << std::endl;
                    throw std::runtime_error(err.str());
                }
        }
    }

    return {
        "",
        pos,
        lineno,
        colno
    };
}
