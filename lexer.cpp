#include "lang.h"
#include <cassert>

/**
 * Feed a string into the code stream.
 */
void lang::Lexer::input(const std::string& code){
    //code_stream.str("");
    //code_stream.clear();
    assert(code_stream);
    std::cerr << "adding :" << code << std::endl;
    code_stream << std::string(code);
    assert(code_stream);
    eof();
}

/**
 * Checks if the stream has reached the end.
 */
bool lang::Lexer::eof(){
    if (!code_stream){
        std::cerr << "code_stream is false" << std::endl;
        return true;
    }
    if (code_stream.eof()){
        std::cerr << "code_stream reached eof" << std::endl;
        return true;
    }
    std::cerr << "checking code_stream peeks eof" << std::endl;
    return code_stream.peek() == EOF;
}

lang::LexToken lang::Lexer::make_tok(const std::string value) const {
    return {value, pos, lineno, colno};
}

/**
 * Scan a single character from the stream.
 */
lang::LexToken lang::Lexer::scan_char(){
    pos++;
    colno++;
    return make_tok(std::string(1, code_stream.get()));
}

static bool valid_name_char(char c){
    return isalnum(c) || c == UNDERSCORE_C;
}

/**
 * name ::= [a-zA-Z_] [a-zA-Z_]*
 */
lang::LexToken lang::Lexer::scan_name(){
    std::string name;
    do {
        name += code_stream.get();
        pos++;
        colno++;
    } while (valid_name_char(code_stream.peek()));
    return make_tok(name);
}

/**
 * int ::= \d+
 */
lang::LexToken lang::Lexer::scan_int(){
    std::string num;
    do {
        num += code_stream.get();
        pos++;
        colno++;
    } while (isdigit(code_stream.peek()));
    return make_tok(num);
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
                        colno = 0;
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
