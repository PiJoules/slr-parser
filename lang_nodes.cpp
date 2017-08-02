#include "nodes.h"
#include <sstream>

static const std::string INDENT = "    ";

/**
 * Module
 */ 
std::vector<std::string> lang::Module::lines() const {
    std::vector<std::string> v;
    for (const ModuleStmt* node : body_){
        for (const std::string line : node->lines()){
            v.push_back(line);
        }
    }
    return v;
}

lang::Module::~Module(){
    for (const ModuleStmt* stmt : body_){
        delete stmt;
    }
}

/**
 * FuncDef Module statement
 */ 
lang::FuncDef::FuncDef(const std::string& func_name, std::vector<FuncStmt*>& func_suite):
    func_name_(func_name),
    func_suite_(func_suite){}

lang::FuncDef::~FuncDef(){
    for (const FuncStmt* stmt : func_suite_){
        delete stmt;
    }
}

const std::vector<lang::FuncStmt*>& lang::FuncDef::suite() const {
    return func_suite_;
}

std::vector<std::string> lang::FuncDef::lines() const {
    std::vector<std::string> v;

    // TODO: Handle arguments in the string also
    std::ostringstream line1;
    line1 << "def " << func_name_ << "():";
    v.push_back(line1.str());

    for (const FuncStmt* stmt : func_suite_){
        for (std::string& stmt_line : stmt->lines()){
            v.push_back(INDENT + stmt_line);
        }
    }

    return v;
}

/**
 * Expression Statement
 */ 
lang::ExprStmt::ExprStmt(Expr* expr): expr_(expr){}

std::vector<std::string> lang::ExprStmt::lines() const {
    std::vector<std::string> v;
    for (const auto& line : expr_->lines()){
        v.push_back(line);
    }
    return v;
}

lang::ExprStmt::~ExprStmt(){
    delete expr_;
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
lang::BinExpr::BinExpr(Expr* lhs, BinOperator* op, Expr* rhs):
    lhs_(lhs), op_(op), rhs_(rhs){}

lang::BinExpr::~BinExpr(){
    delete lhs_;
    delete op_;
    delete rhs_;
}

std::string lang::BinExpr::value_str() const {
    std::ostringstream s;
    s << lhs_->str() << " " << op_->symbol() << " " << rhs_->str();
    return s.str();
}

/**
 * Unary expression
 */ 
lang::UnaryExpr::UnaryExpr(Expr* expr, UnaryOperator* op): expr_(expr), op_(op){}

lang::UnaryExpr::~UnaryExpr(){
    delete expr_;
    delete op_;
}

std::string lang::UnaryExpr::value_str() const {
    std::ostringstream s;
    s << op_->symbol() << expr_->str();
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
