#ifndef _CPP_NODES_H
#define _CPP_NODES_H

#include "parser.h"
#include <sstream>
#include <memory>

namespace cppnodes {
    // Base node representing a whole .c file
    class Module: public parsing::Visitable<Module> {
        private:
            std::vector<std::shared_ptr<Node>> body_;

        public:
            Module(const std::vector<std::shared_ptr<parsing::Node>>& body): body_(body){}
            std::vector<std::string> lines() const;

            void prepend(std::shared_ptr<Node>);
    };

    // Base Expression node
    class Expr: public virtual parsing::SimpleNode {};

    class Stmt: public virtual parsing::Node {};

    // One line statement
    class SimpleStmt: public virtual Stmt, public virtual parsing::SimpleNode {};

    // Multi line statement
    class CompoundStmt: public virtual Stmt {};

    class IfStmt: public CompoundStmt, public parsing::Visitable<IfStmt> {
        private:
            std::shared_ptr<Expr> cond_;
            std::vector<std::shared_ptr<Node>> body_;

        public:
            IfStmt(std::shared_ptr<Expr> cond, const std::vector<std::shared_ptr<Node>>& body): 
                cond_(cond), body_(body){}
            std::vector<std::string> lines() const override;

            std::shared_ptr<Expr> cond() const { return cond_; }
            const std::vector<std::shared_ptr<Node>> body() const { return body_; }
    };

    // Variable declaration
    class VarDecl: public virtual parsing::SimpleNode {};

    /**
     * base<template args> varname;
     *
     * The template args will usually be a type(name) or variable.
     */
    class Type: public parsing::SimpleNode, public parsing::Visitable<Type> {
        private:
            std::shared_ptr<Node> base_;
            std::vector<std::shared_ptr<Node>> template_args_;

        public:
            Type(std::shared_ptr<Node> base): base_(base){}
            Type(std::shared_ptr<Node> base, const std::vector<std::shared_ptr<Node>>& template_args): 
                base_(base), template_args_(template_args){}
            Type(std::shared_ptr<Node> base, std::initializer_list<std::shared_ptr<Node>> template_args): 
                base_(base), template_args_(template_args){}

            std::string line() const override {
                std::string line = base_->str();

                if (!template_args_.empty()){
                    line += "<";

                    std::vector<std::string> arg_strs;
                    for (std::shared_ptr<Node> arg : template_args_){
                        arg_strs.push_back(arg->str());
                    }
                    line += join(arg_strs, ",");

                    line += ">";
                }
                
                return line;
            }

            const std::vector<std::shared_ptr<Node>>& template_args() const { return template_args_; }
    };

    // int x;
    class RegVarDecl: public VarDecl, public parsing::Visitable<RegVarDecl> {
        private:
            std::string name_;
            std::shared_ptr<Type> type_;

        public:
            RegVarDecl(const char* name, std::shared_ptr<Type> type): name_(name), type_(type){}
            RegVarDecl(const std::string& name, std::shared_ptr<Type> type): name_(name), type_(type){}

            std::string line() const override {
                return type_->line() + " " + name_;
            }
    };

    //// int (*func)(int arg1, int arg2)
    //class FuncDecl: public VarDecl, public parsing::Visitable<FuncDecl> {};

    //// struct Person person;
    //class StructDecl: public VarDecl, public parsing::Visitable<StructDecl> {};

    //// enum Color color;
    //class EnumDecl: public VarDecl {}, public parsing::Visitable<EnumDecl> {};

    class FuncDef: public CompoundStmt, public parsing::Visitable<FuncDef> {
        private:
            std::string name_;
            std::string type_;
            std::vector<std::shared_ptr<VarDecl>> args_;
            std::vector<std::shared_ptr<Node>> body_;

        public:
            FuncDef(const std::string&, const std::string&, 
                    const std::vector<std::shared_ptr<VarDecl>>&,
                    const std::vector<std::shared_ptr<Node>>&);
            std::vector<std::string> lines() const override;
    };

    class Name: public Expr, public parsing::Visitable<Name> {
        private:
            std::string id_;

        public:
            Name(const char*);
            Name(const std::string&);
            std::string line() const;
    };

    class Int: public Expr, public parsing::Visitable<Int> {
        private:
            int val_;

        public:
            Int(int val): val_(val){}
            std::string line() const override { 
                std::ostringstream out;
                out << val_;
                return out.str(); 
            }
    };

    class String: public Expr, public parsing::Visitable<String> {
        private:
            std::string value_;

        public:
            String(const std::string&);
            std::string line() const override;
    };

    class Call: public Expr, public parsing::Visitable<Call> {
        private:
            std::shared_ptr<Expr> func_;
            std::vector<std::shared_ptr<Expr>> args_;

        public:
            Call(std::shared_ptr<Expr> func, const std::vector<std::shared_ptr<Expr>>& args): 
                func_(func), args_(args){}
            std::string line() const override;
    };

    class BinOperator: public virtual parsing::SimpleNode {
        public:
            virtual std::string symbol() const = 0;
            std::string line() const override { return symbol(); }
    };
    class Add: public BinOperator, public parsing::Visitable<Add> {
        public:
            std::string symbol() const { return "+"; }
    };
    class Sub: public BinOperator, public parsing::Visitable<Sub> {
        public:
            std::string symbol() const { return "-"; }
    };
    class Div: public BinOperator, public parsing::Visitable<Div> {
        public:
            std::string symbol() const { return "/"; }
    };
    class Mul: public BinOperator, public parsing::Visitable<Mul> {
        public:
            std::string symbol() const { return "*"; }
    };
    
    class Eq: public BinOperator, public parsing::Visitable<Eq> {
        public:
            std::string symbol() const { return "=="; }
    };
    class Ne: public BinOperator, public parsing::Visitable<Ne> {
        public:
            std::string symbol() const { return "!="; }
    };
    class Lt: public BinOperator, public parsing::Visitable<Lt> {
        public:
            std::string symbol() const { return "<"; }
    };
    class Gt: public BinOperator, public parsing::Visitable<Gt> {
        public:
            std::string symbol() const { return ">"; }
    };
    class Lte: public BinOperator, public parsing::Visitable<Lte> {
        public:
            std::string symbol() const { return "<="; }
    };
    class Gte: public BinOperator, public parsing::Visitable<Gte> {
        public:
            std::string symbol() const { return ">="; }
    };

    class BinExpr: public Expr, public parsing::Visitable<BinExpr> {
        private:
            std::shared_ptr<Expr> lhs_;
            std::shared_ptr<BinOperator> op_;
            std::shared_ptr<Expr> rhs_;

        public:
            BinExpr(std::shared_ptr<Expr> lhs, std::shared_ptr<BinOperator> op, 
                    std::shared_ptr<Expr> rhs): 
                lhs_(lhs), op_(op), rhs_(rhs){}
            std::string line() const override {
                return lhs_->line() + " " + op_->symbol() + " " + rhs_->line();
            }

            std::shared_ptr<Expr> lhs() const { return lhs_; }
            std::shared_ptr<BinOperator> op() const { return op_; }
            std::shared_ptr<Expr> rhs() const { return rhs_; }
    };

    class ReturnStmt: public SimpleStmt, public parsing::Visitable<ReturnStmt> {
        private:
            std::shared_ptr<Expr> expr_;

        public:
            ReturnStmt(std::shared_ptr<Expr> expr);
            std::string line() const override;
    };

    class ExprStmt: public SimpleStmt, public parsing::Visitable<ExprStmt> {
        private:
            std::shared_ptr<Expr> expr_;

        public:
            ExprStmt(std::shared_ptr<Expr>);
            std::string line() const override;
    };

    class Assign: public SimpleStmt {
        private:
            std::shared_ptr<VarDecl> var_decl_;
            std::shared_ptr<Expr> expr_;

        public:
            Assign(std::shared_ptr<VarDecl> var_decl, std::shared_ptr<Expr> expr): 
                var_decl_(var_decl), expr_(expr){}

            std::shared_ptr<VarDecl> var_decl() const { return var_decl_; }
            std::shared_ptr<Expr> expr() const { return expr_; }
            std::string line() const override { 
                return var_decl_->line() + " = " + expr_->line() + ";";
            }
    };

    /**
     * Macros
     */ 
    class Macro: public virtual parsing::Node {};

    // Single line macro
    class SimpleMacro: public virtual Macro, public virtual parsing::SimpleNode {};

    class Include: public SimpleMacro, public parsing::Visitable<Include> {
        private:
            std::string name_;

        public:
            Include(const std::string&);
            std::string line() const override;
    };

    // Define without the second argument
    class SimpleDefine: public SimpleMacro, public parsing::Visitable<SimpleDefine> {
        private:
            std::string name_;

        public:
            SimpleDefine(const std::string&);
            std::string line() const override;
    };

    class Ifndef: public SimpleMacro, public parsing::Visitable<Ifndef> {
        private:
            std::string name_;

        public:
            Ifndef(const std::string&);
            std::string line() const override;
    };

    class Endif: public SimpleMacro, public parsing::Visitable<Endif> {
        public:
            Endif();
            std::string line() const;
    };
}

#endif
