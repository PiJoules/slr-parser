#ifndef _CPP_NODES_H
#define _CPP_NODES_H

#include "nodes.h"
#include <sstream>
#include <memory>

namespace cppnodes {
    // Base node representing a whole .c file
    class Module: public lang::Visitable<Module> {
        private:
            std::vector<lang::Node*> body_;

        public:
            Module(const std::vector<lang::Node*>&);
            std::vector<std::string> lines() const;
            ~Module();

            void prepend(Node*);
    };

    // Base Expression node
    class Expr: public lang::Visitable<Expr> {
        public:
            // All expressions can be written on one line
            virtual std::string value_str() const = 0;
            std::vector<std::string> lines() const;
    };

    class Stmt: public lang::Visitable<Stmt> {};

    // One line statement
    class SimpleStmt: public Stmt {
        public:
            virtual std::string value_str() const = 0;
            std::vector<std::string> lines() const;
    };

    // Multi line statement
    class CompoundStmt: public Stmt {};

    class IfStmt: public CompoundStmt {
        private:
            Expr* cond_;
            std::vector<Node*> body_;

        public:
            IfStmt(Expr* cond, std::vector<Node*>& body): cond_(cond), body_(body){}
            ~IfStmt(){
                for (Node* stmt : body_){
                    delete stmt;
                }
                delete cond_;
            }
            std::vector<std::string> lines() const override;

            Expr* cond() const { return cond_; }
            const std::vector<Node*> body() const { return body_; }
    };

    // Variable declaration
    class VarDecl: public lang::Visitable<VarDecl> {
        public:
            virtual std::string line() const = 0;
            std::vector<std::string> lines() const { return {line()}; }
    };

    /**
     * base<template args> varname;
     *
     * The template args will usually be a type(name) or variable.
     */
    class Type: public lang::SimpleNode, public lang::Visitable<Type> {
        private:
            Node* base_;
            std::vector<Node*> template_args_;

        public:
            Type(Node* base): base_(base){}
            Type(Node* base, const std::vector<Node*>& template_args): 
                base_(base), template_args_(template_args){}
            Type(Node* base, std::initializer_list<Node*> template_args): 
                base_(base), template_args_(template_args){}

            ~Type(){
                delete base_;
                for (Node* arg : template_args_){
                    delete arg;
                }
            }

            std::string line() const override {
                std::string line = base_->str();

                if (!template_args_.empty()){
                    line += "<";

                    std::vector<std::string> arg_strs;
                    for (Node* arg : template_args_){
                        arg_strs.push_back(arg->str());
                    }
                    line += join(arg_strs, ",");

                    line += ">";
                }
                
                return line;
            }

            const std::vector<Node*>& template_args() const { return template_args_; }
    };

    // int x;
    class RegVarDecl: public VarDecl {
        private:
            std::string name_;
            Type* type_;

        public:
            RegVarDecl(const char* name, Type* type): name_(name), type_(type){}
            RegVarDecl(const std::string& name, Type* type): name_(name), type_(type){}

            ~RegVarDecl(){
                delete type_;
            }

            std::string line() const override {
                return type_->line() + " " + name_;
            }
    };

    // int (*func)(int arg1, int arg2)
    class FuncDecl: public VarDecl {};

    // struct Person person;
    class StructDecl: public VarDecl {};

    // enum Color color;
    class EnumDecl: public VarDecl {};

    class FuncDef: public CompoundStmt {
        private:
            std::string name_;
            std::string type_;
            std::vector<VarDecl*> args_;
            std::vector<lang::Node*> body_;

        public:
            FuncDef(const std::string&, const std::string&, 
                    const std::vector<VarDecl*>&,
                    const std::vector<lang::Node*>&);
            ~FuncDef();
            std::vector<std::string> lines() const;
    };

    class Name: public Expr {
        private:
            std::string id_;

        public:
            Name(const char*);
            Name(const std::string&);
            std::string value_str() const;
    };

    class Int: public Expr {
        private:
            int val_;

        public:
            Int(int val): val_(val){}
            std::string value_str() const { 
                std::ostringstream out;
                out << val_;
                return out.str(); 
            }
    };

    class String: public Expr {
        private:
            std::string value_;

        public:
            String(const std::string&);
            std::string value_str() const;
    };

    class Call: public Expr {
        private:
            Expr* func_;
            std::vector<Expr*> args_;

        public:
            Call(Expr*, std::vector<Expr*>&);
            ~Call();
            std::string value_str() const;
    };

    class BinOperator: public lang::Visitable<BinOperator> {
        public:
            virtual std::string symbol() const = 0;
            std::vector<std::string> lines() const {
                std::vector<std::string> v = {symbol()};
                return v;
            }
    };
    class Add: public BinOperator {
        public:
            std::string symbol() const { return "+"; }
    };
    class Sub: public BinOperator {
        public:
            std::string symbol() const { return "-"; }
    };
    class Div: public BinOperator {
        public:
            std::string symbol() const { return "/"; }
    };
    class Mul: public BinOperator {
        public:
            std::string symbol() const { return "*"; }
    };
    
    class Eq: public BinOperator {
        public:
            std::string symbol() const { return "=="; }
    };
    class Ne: public BinOperator {
        public:
            std::string symbol() const { return "!="; }
    };
    class Lt: public BinOperator {
        public:
            std::string symbol() const { return "<"; }
    };
    class Gt: public BinOperator {
        public:
            std::string symbol() const { return ">"; }
    };
    class Lte: public BinOperator {
        public:
            std::string symbol() const { return "<="; }
    };
    class Gte: public BinOperator {
        public:
            std::string symbol() const { return ">="; }
    };

    class BinExpr: public Expr {
        private:
            Expr* lhs_;
            BinOperator* op_;
            Expr* rhs_;

        public:
            BinExpr(Expr* lhs, BinOperator* op, Expr* rhs): lhs_(lhs), op_(op), rhs_(rhs){}
            std::string value_str() const override {
                std::string s = lhs_->str() + " " + op_->symbol() + " " + rhs_->str();
                return s;
            }
            ~BinExpr(){
                delete lhs_;
                delete op_;
                delete rhs_;
            }

            Expr* lhs() const { return lhs_; }
            BinOperator* op() const { return op_; }
            Expr* rhs() const { return rhs_; }
    };

    class ReturnStmt: public SimpleStmt {
        private:
            Expr* expr_;

        public:
            ReturnStmt(Expr* expr);
            ~ReturnStmt();
            std::string value_str() const;
    };

    class ExprStmt: public SimpleStmt {
        private:
            Expr* expr_;

        public:
            ExprStmt(Expr*);
            ~ExprStmt();
            std::string value_str() const;
    };

    class Assign: public SimpleStmt {
        private:
            VarDecl* var_decl_;
            Expr* expr_;

        public:
            Assign(VarDecl* var_decl, Expr* expr): var_decl_(var_decl), expr_(expr){}
            ~Assign(){
                delete var_decl_;
                delete expr_;
            }

            VarDecl* var_decl() const { return var_decl_; }
            Expr* expr() const { return expr_; }
            std::string value_str() const { 
                return var_decl_->str() + " = " + expr_->value_str() + ";";
            }
    };

    /**
     * Macros
     */ 
    class Macro: public lang::Visitable<Macro> {};

    // Single line macro
    class SimpleMacro: public Macro {
        public:
            virtual std::string value_str() const = 0;
            std::vector<std::string> lines() const;
    };

    class Include: public SimpleMacro {
        private:
            std::string name_;

        public:
            Include(std::string&);
            std::string value_str() const;
    };

    // Define without the second argument
    class SimpleDefine: public SimpleMacro {
        private:
            std::string name_;

        public:
            SimpleDefine(std::string&);
            std::string value_str() const;
    };

    class Ifndef: public SimpleMacro {
        private:
            std::string name_;

        public:
            Ifndef(std::string&);
            std::string value_str() const;
    };

    class Endif: public SimpleMacro {
        public:
            Endif();
            std::string value_str() const;
    };
}

#endif
