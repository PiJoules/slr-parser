#ifndef _NODES_H
#define _NODES_H

#include "parser.h"

namespace lang {
    class ModuleStmt: public parsing::Node {};
    class FuncStmt: public parsing::Node {};
    class SimpleFuncStmt: public FuncStmt {};
    class Expr: public parsing::Node {
        public:
            // The string representation of the value this expression holds
            virtual std::string value_str() const;

            std::vector<std::string> lines() const;
    };

    class BinOperator: public parsing::Node {
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

    // TODO: Maybe we can merge this and the binary operator class
    // if they don't really end up having different logic in the long run.
    class UnaryOperator: public parsing::Node {
        public:
            virtual std::string symbol() const = 0;
            std::vector<std::string> lines() const {
                std::vector<std::string> v = {symbol()};
                return v;
            }
    };
    class USub: public UnaryOperator {
        public:
            std::string symbol() const { return "-"; }
    };

    class Int: public Expr {
        private:
            int value_;

        public:
            Int(const std::string&);
            Int(int);
            std::string value_str() const;
    };

    class NameExpr: public Expr {
        private:
            std::string name_;

        public:
            NameExpr(const std::string&);
            std::string value_str() const;
    };

    class BinExpr: public Expr {
        private:
            Expr* lhs_;
            BinOperator* op_;
            Expr* rhs_;

        public:
            BinExpr(Expr*, BinOperator*, Expr*);
            std::string value_str() const;
            ~BinExpr();
    };

    class UnaryExpr: public Expr {
        private: 
            Expr* expr_;
            UnaryOperator* op_;

        public:
            UnaryExpr(Expr*, UnaryOperator*);
            std::string value_str() const;
            ~UnaryExpr();
    };

    class ExprStmt: public SimpleFuncStmt {
        private:
            Expr* expr_;

        public:
            ExprStmt(Expr*);
            std::vector<std::string> lines() const;
            ~ExprStmt();
    };

    class FuncDef: public ModuleStmt {
        private:
            std::string func_name_;
            std::vector<FuncStmt*> func_suite_;

        public:
            FuncDef(const std::string&, std::vector<FuncStmt*>&);
            const std::vector<FuncStmt*>& suite() const;
            std::vector<std::string> lines() const;
            ~FuncDef();
    };

    class Newline: public ModuleStmt {
        public:
            std::vector<std::string> lines() const;
    };

    class Module: public parsing::Node {
        private:
            std::vector<ModuleStmt*> body_;

        public:
            Module(std::vector<ModuleStmt*>& body): body_(body){}
            const std::vector<ModuleStmt*>& body() const { return body_; }
            std::vector<std::string> lines() const;
            ~Module();
    };
}

#endif
