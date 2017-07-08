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
lang::FuncDef::FuncDef(const std::string& func_name, const std::vector<FuncStmt*>& func_suite):
    func_name_(func_name),
    func_suite_(func_suite){}

std::vector<std::string> lang::FuncDef::lines() const {
    // TODO: Add the indentations for the lines based off the func_suite
    std::vector<std::string> v;
    return v;
}

lang::FuncDef::~FuncDef(){
    for (lang::FuncStmt* stmt : func_suite_){
        delete stmt;
    }
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

/**
 * Expression Statement
 */ 
lang::ExprStmt::ExprStmt(Expr* expr): expr_(expr){}

lang::ExprStmt::~ExprStmt(){
    delete expr_;
}

std::vector<std::string> lang::ExprStmt::lines() const {
    std::vector<std::string> v;
    for (const auto& line : expr_->lines()){
        v.push_back(line);
    }
    return v;
}

/**
 * Expression
 */ 
std::vector<std::string> lang::Expr::lines() const {
    std::vector<std::string> v = {value_str()};
    return v;
}

std::string lang::Expr::value_str() const {
    return "";
}

/**
 * BinExpr
 */ 
lang::BinExpr::BinExpr(Expr* lhs, BinOperator& op, Expr* rhs):
    lhs_(lhs), op_(op), rhs_(rhs){}

lang::BinExpr::~BinExpr(){
    delete lhs_;
    delete rhs_;
}

std::string lang::BinExpr::value_str() const {
    std::ostringstream s;
    s << lhs_->str() << " " << op_.str() << " " << rhs_->str();
    return s.str();
}

/**
 * Name Expression
 */ 
lang::NameExpr::NameExpr(const std::string& name): name_(name){}

std::string lang::NameExpr::value_str() const {
    return name_;
}

/**
 * Int expression
 */ 
lang::Int::Int(int value): value_(value){}

lang::Int::Int(const std::string& value): value_(std::stoi(value)){}

std::string lang::Int::value_str() const {
    std::ostringstream s;
    s << value_;
    return s.str();
}
