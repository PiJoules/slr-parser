#include "nodes.h"
#include <sstream>
#include <algorithm>

static const std::string INDENT = "    ";


/**
 * Module
 */ 
std::vector<std::string> lang::Module::lines() const {
    std::vector<std::string> v;
    for (const Node* node : body_){
        for (const std::string line : node->lines()){
            v.push_back(line);
        }
    }
    return v;
}

lang::Module::~Module(){
    for (const Node* stmt : body_){
        delete stmt;
    }
}

/**
 * Function args
 */ 
std::string lang::FuncArgs::line() const {
    // Func positional args  
    std::vector<std::string> arg_strs;
    for (VarDecl* arg : pos_args_){
        arg_strs.push_back(arg->line());
    }

    // varargs 
    if (has_varargs()){
        arg_strs.push_back("*" + varargs_name_);
    }

    // keyword args
    for (Assign* arg : keyword_args_){
        arg_strs.push_back(arg->line());
    }

    // kwargs
    if (has_kwargs()){
        arg_strs.push_back("**" + kwargs_name_);
    }

    return join(arg_strs, ", ");
}

/**
 * FuncDef Module statement
 */ 
lang::FuncDef::~FuncDef(){
    delete args_;
    delete return_type_decl_;
    for (const Node* stmt : func_suite_){
        delete stmt;
    }
}

std::vector<std::string> lang::FuncDef::lines() const {
    std::vector<std::string> v;

    std::ostringstream line1;
    line1 << "def " << func_name_ << "(" << args_->line() << ") -> ";

    // Return type 
    line1 << return_type_decl_->value_str();

    line1 << ":";

    v.push_back(line1.str());

    for (const Node* stmt : func_suite_){
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

std::string lang::ExprStmt::line() const {
    return expr_->str();
}

lang::ExprStmt::~ExprStmt(){
    delete expr_;
}

/**
 * Return statement
 */
lang::ReturnStmt::ReturnStmt(Expr* expr): expr_(expr){}

std::string lang::ReturnStmt::line() const {
    return "return " + expr_->str();
}

lang::ReturnStmt::~ReturnStmt(){
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
 * Function Call
 */ 
lang::Call::Call(Expr* func): func_(func){}
lang::Call::Call(Expr* func, const std::vector<Expr*>& args): func_(func), args_(args){}

lang::Call::~Call(){
    delete func_;
    for (Expr* arg : args_){
        delete arg;
    }
}

std::string lang::Call::value_str() const {
    std::string joined_args;
    if (!args_.empty()){
        joined_args += args_.front()->value_str();
    }
    for (auto it = args_.begin() + 1 ; it < args_.end(); ++it){
        joined_args += ", " + (*it)->value_str();
    }
    return func_->str() + "(" + joined_args + ")";
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
 * String literal
 */ 
lang::String::String(const std::string& value): value_(value){}
std::string lang::String::value_str() const {
    return "\"" + value_ + "\"";
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

/**
 * If statement
 */ 
std::vector<std::string> lang::IfStmt::lines() const {
    std::vector<std::string> stmt_lines;

    std::string line1 = "if " + cond_->value_str() + ":";
    stmt_lines.push_back(line1);

    for (FuncStmt* stmt : body_){
        for (std::string& stmt_line : stmt->lines()){
            stmt_lines.push_back(INDENT + stmt_line);
        }
    }

    return stmt_lines;
}

std::shared_ptr<lang::LangType> lang::StarArgsTypeDecl::as_type() const { 
    return std::shared_ptr<LangType>(new StarArgsType); 
}
std::shared_ptr<lang::LangType> lang::NameTypeDecl::as_type() const { 
    return std::shared_ptr<LangType>(new NameType(name_)); 
}
std::shared_ptr<lang::LangType> lang::FuncTypeDecl::as_type() const {
    std::vector<std::shared_ptr<LangType>> args;
    for (TypeDecl* arg : args_){
        args.push_back(arg->as_type());
    }
    return std::shared_ptr<LangType>(new FuncType(return_type_->as_type(), args,
                                                  has_varargs_, has_kwargs_));
}
