#include "lang.h"

/**
 * Exceptions
 */
const char* lang::IndentationError::what() const throw() {
    std::ostringstream err;
    err << std::runtime_error::what();
    err << "Unexpected indentation on line " << lineno_ << ".";
    return err.str().c_str();
}

/********** Debugging utilities **********/ 

/**
 * String conversion
 */
std::string lang::str(const LexToken& tok){
    std::ostringstream s;
    s << "{" << "symbol: " << tok.symbol << ", value: '" << tok.value 
      << "', pos: " << tok.pos << ", lineno: " << tok.lineno << ", colno: "
      << tok.colno << "}";  
    return s.str();
}

std::string lang::str(const lang::production_t& production){
    std::ostringstream s;
    std::size_t len = production.size();
    int end = static_cast<int>(len) - 1;
    for (int i = 0; i < end; ++i){
        s << production[i] << " ";
    }
    if (len){
        s << production.back();
    }
    return s.str();
}

std::string lang::str(const ParseRule& rule){
    std::ostringstream s;
    s << rule.rule << " -> " << str(rule.production);
    return s.str();
}

std::string lang::str(const lr_item_t& lr_item){
    ParseRule prod_rule = lr_item.first;
    int pos = lr_item.second;
    std::ostringstream s;
    s << prod_rule.rule << " : ";

    // Before dot
    for (int i = 0; i < pos; ++i){
        s << prod_rule.production[i] << " ";
    }

    s << ". ";

    // After dot 
    int len = static_cast<int>(prod_rule.production.size());
    for (int i = pos; i < len; ++i){
        s << prod_rule.production[i] << " ";
    }

    return s.str();
}

std::string lang::str(const ParseInstr::Action& action){
    switch (action){
        case ParseInstr::SHIFT: return "shift";
        case ParseInstr::REDUCE: return "reduce";
        case ParseInstr::GOTO: return "goto";
        case ParseInstr::ACCEPT: return "accept";
        default: return "";
    }
}
