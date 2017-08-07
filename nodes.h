#ifndef _NODES_H
#define _NODES_H 

#include <vector>
#include <string>

#include <typeinfo>
#include <stdexcept>
#include <sstream>

namespace lang {

    class NodeVisitor {
        public:
            virtual ~NodeVisitor(){}
    };

    class Node {
        public:
            virtual void* accept(NodeVisitor&) = 0;
            virtual ~Node(){}

            // lines() returns a vector containing strings that represent 
            // individual lines separated in the code separated by newlines.
            virtual std::vector<std::string> lines() const = 0;

            // The lines joined by newlines
            std::string str() const {
                std::string s;

                const std::vector<std::string>& node_lines = lines();
                if (!node_lines.empty()){
                    s += node_lines.front();
                }
                for (auto it = node_lines.begin() + 1; it < node_lines.end(); ++it){
                    s += "\n" + *it;
                }

                return s;
            }
    };
    
    template <typename VisitedNode>
    class Visitor {
        public:
            virtual void* visit(VisitedNode*) = 0;
    };

    template <typename DerivedNode>
    class Visitable: public virtual Node {
        public:
            void* accept(NodeVisitor& base_visitor){
                try {
                    Visitor<DerivedNode>& visitor = dynamic_cast<Visitor<DerivedNode>&>(base_visitor);
                    return visitor.visit(static_cast<DerivedNode*>(this));
                } catch (const std::bad_cast& e){
                    std::ostringstream err;
                    err << "Bad cast thrown in: " << typeid(DerivedNode).name() << std::endl;
                    throw std::runtime_error(err.str());
                }
            }
    };

    class ModuleStmt: public virtual Node {};
    class FuncStmt: public virtual Node {};

    class SimpleFuncStmt: public virtual FuncStmt {
        public:
            virtual std::string line() const = 0;
            
            std::vector<std::string> lines() const {
                std::vector<std::string> v;
                v.push_back(line());
                return v;
            };
    };

    class Expr: public virtual Node {
        public:
            // The string representation of the value this expression holds
            virtual std::string value_str() const;

            std::vector<std::string> lines() const;
    };

    class Call: public Expr, public Visitable<Call> {
        private:
            Expr* func_;
            std::vector<Expr*> args_;

        public:
            Call(Expr*);
            Call(Expr*, const std::vector<Expr*>&);
            ~Call();

            Expr* func() const { return func_; }
            const std::vector<Expr*>& args() const { return args_; }

            std::string value_str() const override;
    };

    class BinOperator: public Visitable<BinOperator> {
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
    class UnaryOperator: public Visitable<UnaryOperator> {
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

    class Int: public Expr, public Visitable<Int> {
        private:
            int value_;

        public:
            Int(const std::string&);
            Int(int);
            std::string value_str() const;
            int value() const { return value_; }
    };

    class NameExpr: public Expr, public Visitable<NameExpr> {
        private:
            std::string name_;

        public:
            NameExpr(const std::string&);
            std::string value_str() const;
            std::string name() const { return name_; }
    };

    class String: public Expr, public Visitable<String> {
        private:
            std::string value_;

        public:
            String(const std::string&);
            std::string value_str() const;
            std::string value() const { return value_; }
    };

    class BinExpr: public Expr, public Visitable<BinExpr> {
        private:
            Expr* lhs_;
            BinOperator* op_;
            Expr* rhs_;

        public:
            BinExpr(Expr*, BinOperator*, Expr*);
            std::string value_str() const;
            ~BinExpr();
    };

    class UnaryExpr: public Expr, public Visitable<UnaryExpr> {
        private: 
            Expr* expr_;
            UnaryOperator* op_;

        public:
            UnaryExpr(Expr*, UnaryOperator*);
            std::string value_str() const;
            ~UnaryExpr();
    };

    class ExprStmt: public SimpleFuncStmt, public Visitable<ExprStmt> {
        private:
            Expr* expr_;

        public:
            ExprStmt(Expr*);
            std::string line() const override;
            ~ExprStmt();
            Expr* expr() const { return expr_; }
    };

    class ReturnStmt: public SimpleFuncStmt, public Visitable<ReturnStmt> {
        private:
            Expr* expr_;

        public:
            ReturnStmt(Expr*);
            std::string line() const;
            ~ReturnStmt();

            Expr* expr() const { return expr_; }
    };

    class TypeDecl: public virtual Node {
        public:
            virtual std::string value_str() const = 0;
            
            std::vector<std::string> lines() const {
                std::vector<std::string> v;
                v.push_back(value_str());
                return v;
            };
    };

    class NameTypeDecl: public TypeDecl, public Visitable<NameTypeDecl> {
        private:
            std::string name_;

        public:
            NameTypeDecl(std::string& name): name_(name){}
            std::string name() const { return name_; }
            std::string value_str() const override { return name_; }
    };

    class VarDecl: public Visitable<VarDecl> {
        private:
            std::string name_;
            TypeDecl* type_;

        public:
            VarDecl(std::string& name, TypeDecl* type): name_(name), type_(type){}
            ~VarDecl(){
                delete type_;
            }

            std::string name() const { return name_; }
            TypeDecl* type() const { return type_; }

            std::vector<std::string> lines() const {
                std::vector<std::string> v;
                std::string line = name_ + ": " + type_->value_str();
                return v;
            }
    };

    class FuncDef: public ModuleStmt, public Visitable<FuncDef> {
        private:
            std::string func_name_;
            std::vector<VarDecl*> args_;
            std::vector<FuncStmt*> func_suite_;

        public:
            FuncDef(const std::string&, const std::vector<VarDecl*>&, std::vector<FuncStmt*>&);
            std::vector<std::string> lines() const;
            ~FuncDef();

            const std::vector<FuncStmt*>& suite() const;
            std::string name() const { return func_name_; }
            const std::vector<VarDecl*>& args() const { return args_; }
    };

    class Module: public Visitable<Module> {
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
