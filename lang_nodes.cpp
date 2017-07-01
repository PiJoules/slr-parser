#include "lang.h"

/**
 * Base node
 */ 
std::string lang::Node::str() const {
    std::ostringstream s;
    for (const auto& line : lines()){
        s << line << std::endl;
    }
    return s.str();
}

/**
 * LexTokenWrapper
 */
lang::LexTokenWrapper::LexTokenWrapper(const LexToken& token): token_(token){}

lang::LexToken lang::LexTokenWrapper::token() const { return token_; }

std::vector<std::string> lang::LexTokenWrapper::lines() const { 
    std::vector<std::string> v = {lang::str(token_)};
    return v;
}

lang::LexTokenWrapper::~LexTokenWrapper(){}

/**
 * Module
 */ 
std::vector<std::string> lang::Module::lines() const {
    std::vector<std::string> v;
    for (ModuleStmt* node : body_){
        for (const std::string line : node->lines()){
            v.push_back(line);
        }
    }
    return v;
}

lang::Module::~Module(){
    for (const auto node : body_){
        delete node;
    }
}

/**
 * Module statement
 */
std::vector<std::string> lang::ModuleStmt::lines() const {
    std::vector<std::string> v;
    return v;
}

lang::ModuleStmt::~ModuleStmt(){}
