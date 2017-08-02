#ifndef _NODES_H
#define _NODES_H 

#include <vector>
#include <string>

namespace lang {
    
    template <typename Node>
    class NodeVisitor {
        public:
            virtual void* visit(Node*) = 0;
    };

    template <typename DerivedNode>
    class Node {
        public:
            void* accept(NodeVisitor<DerivedNode>& visitor){
                return visitor.visit(static_cast<DerivedNode*>(this));
            }

            // lines() returns a vector containing strings that represent 
            // individual lines separated in the code separated by newlines.
            virtual std::vector<std::string> lines() const = 0;

            // The lines joined by newlines
            std::string str() const;

            virtual ~Node(){}
    };

    class ModuleStmt: public Node<ModuleStmt> {};
    class FuncStmt: public Node<FuncStmt> {};
    class SimpleFuncStmt: public FuncStmt {};
    class Expr: public Node<Expr> {
        public:
            // The string representation of the value this expression holds
            virtual std::string value_str() const;

            std::vector<std::string> lines() const;
    };

    class BinOperator: public Node<BinOperator> {
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
    class UnaryOperator: public Node<UnaryOperator> {
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

    class Module: public Node<Module> {
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
