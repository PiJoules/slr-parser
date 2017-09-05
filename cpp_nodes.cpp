#include "cpp_nodes.h"

const std::string INDENT = "    ";

/**
 * Module
 */
cppnodes::Module::Module(const std::vector<parsing::Node*>& body): body_(body){}

cppnodes::Module::~Module(){
    for (Node* node : body_){
        delete node;
    }
}

std::vector<std::string> cppnodes::Module::lines() const {
    std::vector<std::string> v;
    for (Node* node : body_){
        for (std::string& line : node->lines()){
            v.push_back(line);
        }
    }
    return v;
}

void cppnodes::Module::prepend(Node* node){
    body_.insert(body_.begin(), node);
}

/**
 * If stmt 
 */ 
std::vector<std::string> cppnodes::IfStmt::lines() const {
    std::vector<std::string> stmt_lines;

    std::string line1 = "if (" + cond_->value_str() + "){";
    stmt_lines.push_back(line1);

    for (Node* stmt : body_){
        for (std::string& stmt_line : stmt->lines()){
            stmt_lines.push_back(INDENT + stmt_line);
        }
    }

    stmt_lines.push_back("}");

    return stmt_lines;
}

/**
 * Function definition
 */ 
cppnodes::FuncDef::FuncDef(const std::string& name,
                           const std::string& type, 
                           const std::vector<VarDecl*>& args,
                           const std::vector<parsing::Node*>& body):
    name_(name), type_(type), args_(args), body_(body){}

cppnodes::FuncDef::~FuncDef(){
    for (VarDecl* arg : args_){
        delete arg;
    }

    for (Node* node : body_){
        delete node;
    }
}

std::vector<std::string> cppnodes::FuncDef::lines() const {
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
 * Return statement
 */ 
cppnodes::ReturnStmt::ReturnStmt(Expr* expr): expr_(expr){}

cppnodes::ReturnStmt::~ReturnStmt(){ delete expr_; }

std::string cppnodes::ReturnStmt::value_str() const {
    return "return " + expr_->str() + ";";
}

/**
 * Expression statement
 */ 
cppnodes::ExprStmt::ExprStmt(Expr* expr): expr_(expr){}

cppnodes::ExprStmt::~ExprStmt(){
    delete expr_;
}

std::string cppnodes::ExprStmt::value_str() const {
    return expr_->str() + ";";
}

/**
 * Expression
 *
 * All expressions van be written on one line.
 * The lines returned are just whatever is returned by value_str();
 */  
std::vector<std::string> cppnodes::Expr::lines() const {
    std::vector<std::string> v;
    v.push_back(value_str());
    return v;
}

/**
 * Name expression
 */ 
cppnodes::Name::Name(const std::string& id): id_(id){}
cppnodes::Name::Name(const char* id): id_(id){}

std::string cppnodes::Name::value_str() const { return id_; }

/**
 * String literal
 */ 
cppnodes::String::String(const std::string& value): value_(value){}

std::string cppnodes::String::value_str() const {
    return "\"" + value_ + "\"";
}

/**
 * Function call expression
 */ 
cppnodes::Call::Call(Expr* func, std::vector<Expr*>& args): func_(func), args_(args){}

cppnodes::Call::~Call(){
    delete func_;

    for (Expr* arg : args_){
        delete arg;
    }
}

std::string cppnodes::Call::value_str() const {
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
std::vector<std::string> cppnodes::SimpleStmt::lines() const {
    std::vector<std::string> v;
    v.push_back(value_str());
    return v;
}

/**
 * Simple macro
 */  
std::vector<std::string> cppnodes::SimpleMacro::lines() const {
    std::vector<std::string> v;
    v.push_back(value_str());
    return v;
}

/**
 * Local Include
 */ 
cppnodes::Include::Include(std::string& name): name_(name){}
std::string cppnodes::Include::value_str() const {
    return "#include \"" + name_ + "\"";
}

/**
 * SimpleDefine macro
 */ 
cppnodes::SimpleDefine::SimpleDefine(std::string& name): name_(name){}

std::string cppnodes::SimpleDefine::value_str() const {
    return "#define " + name_;
}

/**
 * Ifndef macro
 */ 
cppnodes::Ifndef::Ifndef(std::string& name): name_(name){}

std::string cppnodes::Ifndef::value_str() const {
    return "#ifndef" + name_;
}

/**
 * Endif macro
 */ 
cppnodes::Endif::Endif(){}

std::string cppnodes::Endif::value_str() const {
    return "#endif";
}
