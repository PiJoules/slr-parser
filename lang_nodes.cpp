#include "lang_nodes.h"
#include <sstream>
#include <algorithm>

/**
 * Module
 */ 
std::vector<std::string> lang::Module::lines() const {
    std::vector<std::string> v;
    for (const std::shared_ptr<ModuleStmt> node : body_){
        for (const std::string line : node->lines()){
            v.push_back(line);
        }
    }
    return v;
}

/**
 * Function args
 */ 
std::string lang::FuncArgs::line() const {
    // Func positional args  
    std::vector<std::string> arg_strs;
    for (std::shared_ptr<VarDecl> arg : pos_args_){
        arg_strs.push_back(arg->line());
    }

    // keyword args
    for (std::shared_ptr<Assign> arg : keyword_args_){
        arg_strs.push_back(arg->line());
    }

    // kwargs
    if (has_varargs_){
        arg_strs.push_back("...");
    }

    return join(arg_strs, ", ");
}

/**
 * FuncDef Module statement
 */ 
std::vector<std::string> lang::FuncDef::lines() const {
    std::vector<std::string> v;

    std::string line1 = "def " + func_name_ + "(" + args_->line() + ") -> ";

    // Return type 
    line1 += return_type_decl_->line();

    line1 += ":";

    v.push_back(line1);

    for (const std::shared_ptr<Node> stmt : func_suite_){
        for (std::string& stmt_line : stmt->lines()){
            v.push_back(INDENT + stmt_line);
        }
    }

    return v;
}

/**
 * Function Call
 */ 
std::string lang::Call::line() const {
    std::string joined_args;
    if (!args_.empty()){
        joined_args += args_.front()->line();
    }
    for (auto it = args_.begin() + 1 ; it < args_.end(); ++it){
        joined_args += ", " + (*it)->line();
    }
    return func_->str() + "(" + joined_args + ")";
}

/**
 * BinExpr
 */ 
std::string lang::BinExpr::line() const {
    return lhs_->line() + " " + op_->symbol() + " " + rhs_->line();
}

/**
 * Unary expression
 */ 
std::string lang::UnaryExpr::line() const {
    return op_->symbol() + expr_->line();
}

/**
 * Name Expression
 */ 
lang::NameExpr::NameExpr(const std::string& name): name_(name){}

std::string lang::NameExpr::line() const {
    return name_;
}

/**
 * String literal
 */ 
lang::String::String(const std::string& value): value_(value){}
std::string lang::String::line() const {
    return "\"" + value_ + "\"";
}

/**
 * Int expression
 */ 
lang::Int::Int(int value): value_(value){}

lang::Int::Int(const std::string& value): value_(std::stoi(value)){}

std::string lang::Int::line() const {
    std::ostringstream s;
    s << value_;
    return s.str();
}

/**
 * If statement
 */ 
std::vector<std::string> lang::IfStmt::lines() const {
    std::vector<std::string> stmt_lines;

    std::string line1 = "if " + cond_->line() + ":";
    stmt_lines.push_back(line1);

    for (std::shared_ptr<FuncStmt> stmt : body_){
        for (std::string& stmt_line : stmt->lines()){
            stmt_lines.push_back(INDENT + stmt_line);
        }
    }

    return stmt_lines;
}

std::shared_ptr<lang::LangType> lang::StarArgsTypeDecl::as_type() const { 
    return std::make_shared<StarArgsType>(); 
}

std::shared_ptr<lang::LangType> lang::NameTypeDecl::as_type() const { 
    return std::make_shared<NameType>(name_); 
}

std::shared_ptr<lang::LangType> lang::FuncTypeDecl::as_type() const {
    std::vector<std::shared_ptr<LangType>> args;
    for (std::shared_ptr<TypeDecl> arg : args_){
        args.push_back(arg->as_type());
    }
    return std::make_shared<FuncType>(return_type_->as_type(), args, has_varargs_);
}

std::shared_ptr<lang::LangType> lang::TupleTypeDecl::as_type() const {
    std::vector<std::shared_ptr<LangType>> content_types;

    for (std::shared_ptr<TypeDecl> decl : contents_){
        content_types.push_back(decl->as_type());
    }

    return std::make_shared<TupleType>(content_types);
}

std::shared_ptr<lang::LangType> lang::StringTypeDecl::as_type() const {
    return std::make_shared<StringType>();
}
