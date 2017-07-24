#include "c_nodes.h"

static const std::string INDENT = "    ";

/**
 * Module
 */
cnodes::Module::Module(const std::vector<parsing::Node*>& body): body_(body){}

cnodes::Module::~Module(){
    for (Node* node : body_){
        delete node;
    }
}

std::vector<std::string> cnodes::Module::lines() const {
    std::vector<std::string> v;
    for (Node* node : body_){
        for (std::string& line : node->lines()){
            v.push_back(line);
        }
    }
    return v;
}

/**
 * Function definition
 */ 
cnodes::FuncDef::FuncDef(const std::string& name,
                         const std::string& type, 
                         const std::vector<VarDecl*>& args,
                         const std::vector<parsing::Node*>& body):
    name_(name), type_(type), args_(args), body_(body){}

cnodes::FuncDef::~FuncDef(){
    for (VarDecl* arg : args_){
        delete arg;
    }

    for (Node* node : body_){
        delete node;
    }
}

std::vector<std::string> cnodes::FuncDef::lines() const {
    std::vector<std::string> v;

    // Prototype
    std::string name_line = type_ + " " + name_ + "(";
    
    // Args 
    if (!args_.empty()){
        name_line += args_.front()->str();
    }
    for (auto it = args_.begin() + 1; it < args_.end(); ++it){
        name_line += ", " + (*it)->str();
    }

    // Open body
    name_line += "){";
    v.push_back(name_line);

    // Body
    for (Node* node : body_){
        for (std::string& line : node->lines()){
            std::string final_line = INDENT + line;
            v.push_back(final_line);
        }
    }

    // Close body
    v.push_back("}");

    return v;
}

/**
 * Regular variable declaration
 */ 
cnodes::RegVarDecl::RegVarDecl(const char* name, const char* type): name_(name), type_(type){}
cnodes::RegVarDecl::RegVarDecl(std::string& name, std::string& type): name_(name), type_(type){}

std::vector<std::string> cnodes::RegVarDecl::lines() const {
    std::vector<std::string> v;
    std::string line = type_ + " " + name_;
    v.push_back(line);
    return v;
}

/**
 * Return statement
 */ 
cnodes::ReturnStmt::ReturnStmt(Expr* expr): expr_(expr){}

cnodes::ReturnStmt::~ReturnStmt(){ delete expr_; }

std::string cnodes::ReturnStmt::value_str() const {
    return "return " + expr_->str() + ";";
}

/**
 * Expression
 *
 * All expressions van be written on one line.
 * The lines returned are just whatever is returned by value_str();
 */  
std::vector<std::string> cnodes::Expr::lines() const {
    std::vector<std::string> v;
    v.push_back(value_str());
    return v;
}

/**
 * Name expression
 */ 
cnodes::Name::Name(std::string& id): id_(id){}
cnodes::Name::Name(const char* id): id_(id){}

std::string cnodes::Name::value_str() const { return id_; }

/**
 * Function call expression
 */ 
cnodes::Call::Call(Expr* func, std::vector<Expr*>& args): func_(func), args_(args){}

cnodes::Call::~Call(){
    delete func_;

    for (Expr* arg : args_){
        delete arg;
    }
}

std::string cnodes::Call::value_str() const {
    std::string line = func_->str() + "(";

    if (!args_.empty()){
        line += args_.front()->str();
    }
    for (auto it = args_.begin() + 1; it < args_.end(); ++it){
        line += ", " + (*it)->str();
    }

    line += ")";
    return line;
}

/**
 * Simple statement
 */ 
std::vector<std::string> cnodes::SimpleStmt::lines() const {
    std::vector<std::string> v;
    v.push_back(value_str());
    return v;
}
