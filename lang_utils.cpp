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

std::string lang::str(const prod_rule_t& prod_rule){
    std::ostringstream s;
    s << prod_rule.first << " -> " << str(prod_rule.second);
    return s.str();
}

std::string lang::str(const lr_item_t& lr_item){
    auto prod_rule = lr_item.first;
    auto pos = lr_item.second;
    std::ostringstream s;
    s << prod_rule.first << " : ";

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
