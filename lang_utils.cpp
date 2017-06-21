#include "lang.h"

/********** Debugging utilities **********/ 

/**
 * String conversion
 */
std::string lang::str(const enum Symbol& symbol){
    switch (symbol){
        // Values
        case int_tok: return "INT";
        case name_tok: return "NAME";

        // Binary ops
        case add_tok: return "ADD";
        case sub_tok: return "SUB";
        case mul_tok: return "MUL";
        case div_tok: return "DIV";

        // Misc 
        case newline_tok: return "NEWLINE";
        case whitespace_tok: return "WS";
        case indent_tok: return "INDENT";
        case dedent_tok: return "DEDENT";
        case eof_tok: return "EOF";
        
        // Parser rules
        case module_rule: return "module";
        case funcdef_rule: return "funcdef";
        case expr_rule: return "expr";

        default:
            return std::to_string(static_cast<int>(symbol));
    }
}

std::string lang::str(const lang::production_t& production){
    std::ostringstream s;
    std::size_t len = production.size();
    int end = static_cast<int>(len) - 1;
    for (int i = 0; i < end; ++i){
        s << str(production[i]) << " ";
    }
    if (len){
        s << str(production.back());
    }
    return s.str();
}

std::string lang::str(const prod_rule_t& prod_rule){
    std::ostringstream s;
    s << str(prod_rule.first) << " -> " << str(prod_rule.second);
    return s.str();
}

std::string lang::str(const lr_item_t& lr_item){
    auto prod_rule = lr_item.first;
    auto pos = lr_item.second;
    std::ostringstream s;
    s << static_cast<int>(prod_rule.first) << " : ";

    // Before dot
    for (int i = 0; i < pos; ++i){
        s << prod_rule.second[i] << " ";
    }

    s << ". ";

    // After dot 
    int len = static_cast<int>(prod_rule.second.size());
    for (int i = pos; i < len; ++i){
        s << prod_rule.second[i] << " ";
    }

    return s.str();
}