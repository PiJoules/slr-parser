#include "cpp_nodes.h"

const std::string INDENT = "    ";

/**
 * Module
 */

std::vector<std::string> cppnodes::Module::lines() const {
    std::vector<std::string> v;
    for (std::shared_ptr<Node> node : body_){
        for (std::string& line : node->lines()){
            v.push_back(line);
        }
    }
    return v;
}

void cppnodes::Module::prepend(std::shared_ptr<Node> node){
    body_.insert(body_.begin(), node);
}

/**
 * If stmt 
 */ 
std::vector<std::string> cppnodes::IfStmt::lines() const {
    std::vector<std::string> stmt_lines;

    std::string line1 = "if (" + cond_->line() + "){";
    stmt_lines.push_back(line1);

    for (std::shared_ptr<Node> stmt : body_){
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
                           const std::vector<std::shared_ptr<VarDecl>>& args,
                           const std::vector<std::shared_ptr<parsing::Node>>& body):
    name_(name), type_(type), args_(args), body_(body){}

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
    for (std::shared_ptr<Node> node : body_){
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
cppnodes::ReturnStmt::ReturnStmt(std::shared_ptr<Expr> expr): expr_(expr){}

std::string cppnodes::ReturnStmt::line() const {
    return "return " + expr_->line() + ";";
}

/**
 * Expression statement
 */ 
cppnodes::ExprStmt::ExprStmt(std::shared_ptr<Expr> expr): expr_(expr){}

std::string cppnodes::ExprStmt::line() const {
    return expr_->line() + ";";
}

/**
 * Name expression
 */ 
cppnodes::Name::Name(const std::string& id): id_(id){}
cppnodes::Name::Name(const char* id): id_(id){}

std::string cppnodes::Name::line() const { return id_; }

/**
 * String literal
 */ 
cppnodes::String::String(const std::string& value): value_(value){}

std::string cppnodes::String::line() const {
    return "\"" + value_ + "\"";
}

/**
 * Function call expression
 */ 
std::string cppnodes::Call::line() const {
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
 * Local Include
 */ 
cppnodes::Include::Include(const std::string& name): name_(name){}
std::string cppnodes::Include::line() const {
    return "#include \"" + name_ + "\"";
}

/**
 * SimpleDefine macro
 */ 
cppnodes::SimpleDefine::SimpleDefine(const std::string& name): name_(name){}

std::string cppnodes::SimpleDefine::line() const {
    return "#define " + name_;
}

/**
 * Ifndef macro
 */ 
cppnodes::Ifndef::Ifndef(const std::string& name): name_(name){}

std::string cppnodes::Ifndef::line() const {
    return "#ifndef" + name_;
}

/**
 * Endif macro
 */ 
cppnodes::Endif::Endif(){}

std::string cppnodes::Endif::line() const {
    return "#endif";
}
