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
 * FuncDef Module statement
 */ 
lang::FuncDef::FuncDef(const std::string& func_name, const FuncSuite* func_suite):
    func_name_(func_name),
    func_suite_(func_suite){}

std::vector<std::string> lang::FuncDef::lines() const {
    // TODO: Add the indentations for the lines based off the func_suite
    std::vector<std::string> v;
    return v;
}

lang::FuncDef::~FuncDef(){
    delete func_suite_;
}

/**
 * Newline Module Statement
 */ 
std::vector<std::string> lang::Newline::lines() const {
    std::ostringstream out;
    out << std::endl;
    std::vector<std::string> v = {out.str()};
    return v;
}

lang::Newline::~Newline(){}
