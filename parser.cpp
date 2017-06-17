#include "lang.h"

void lang::Parser::input(const std::string& code){
    lexer.input(code);
}

/**
 * Returns true if the value of the next token from the lexer matches
 * what we expect.
 */
bool lang::Parser::check_terminal(const std::string& terminal) const {
    return lexer.peek().value == terminal;
}

/**
 * Pop a token from the lexer.
 */
void lang::Parser::accept_terminal(const std::string& terminal) {
    assert(check_terminal(terminal));
    lexer.token();
}


/**
 * module : module_stmt_list
 */
lang::LangNode lang::Parser::parse_module(){
    Module mod;
    mod.body = parse_module_stmt_list();
    return mod;
}

/**
 * module_stmt_list : module_stmt 
 *                  | NEWLINE
 *                  | module_stmt module_stmt_list
 *                  | NEWLINE module_stmt_list
 *                  | empty
 */
std::vector<lang::ModuleStmt> lang::Parser::parse_module_stmt_list(){
    std::vector<ModuleStmt> stmt_lst;

    //while (1){
    //    if (check_module_stmt()){
    //        stmt_lst.push_back(parse_module_stmt());
    //    }
    //    else if (check_terminal("\n")){
    //        accept_terminal();
    //    }
    //    else {
    //        std::ostringstream err;
    //        err << "Unknown terminal '" << 
    //        throw std::runtime_error();
    //    }
    //}

    return stmt_lst;
}
