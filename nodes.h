#ifndef _NODES_H
#define _NODES_H 

#include <vector>
#include <string>

#include <typeinfo>
#include <stdexcept>
#include <sstream>
#include <initializer_list>
#include <memory>
#include <iostream>

#include "utils.h"

const std::string INDENT = "    ";

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

#ifdef DEBUG
                    std::cerr << "visiting " << typeid(DerivedNode).name() << std::endl;
#endif

                    void* result = visitor.visit(static_cast<DerivedNode*>(this));

#ifdef DEBUG 
                    std::cerr << "leaving " << typeid(DerivedNode).name() << std::endl;
#endif

                    return result;
                } catch (const std::bad_cast& e){
                    std::ostringstream err;
                    err << "Bad cast thrown in: " << typeid(DerivedNode).name() << std::endl;
                    err << "Check if your Visitor implementation both inherits from 'Visitor<your node>' and implements 'void* visit(your node*)'." << std::endl;
                    throw std::runtime_error(err.str());
                }
            }
    };

    class ModuleStmt: public virtual Node {};
    class FuncStmt: public virtual Node {};

    class SimpleNode: public virtual Node {
        public:
            virtual std::string line() const = 0;
            
            std::vector<std::string> lines() const {
                std::vector<std::string> v;
                v.push_back(line());
                return v;
            };
    };

    class SimpleFuncStmt: public virtual FuncStmt, public virtual SimpleNode {};

    class LangType;

    // The type of an object
    class TypeDecl: public virtual Node {
        public:
            virtual std::string value_str() const = 0;
            virtual std::shared_ptr<LangType> as_type() const = 0;
            
            std::vector<std::string> lines() const {
                std::vector<std::string> v;
                v.push_back(value_str());
                return v;
            };
    };

    class BaseInferer {
        public:
            virtual ~BaseInferer(){}
    };

    class LangType {
        public:
            virtual TypeDecl* as_type_decl() const = 0;
            virtual ~LangType(){}
            virtual bool equals(const LangType&) const = 0;

            bool operator==(const LangType& other) const { return equals(other); }
            bool operator!=(const LangType& other) const { return !(*this == other); }
    };

    class Expr: public virtual Node {
        public:
            // The string representation of the value this expression holds
            virtual std::string value_str() const = 0;
            virtual std::shared_ptr<LangType> type(BaseInferer&) = 0;

            std::vector<std::string> lines() const;
    };

    template <typename VisitedExpr>
    class Inferer {
        public:
            virtual std::shared_ptr<LangType> infer(VisitedExpr*) = 0;
    };

    template <typename DerivedExpr>
    class VisitableExpr: public virtual Expr {
        public:
            std::shared_ptr<LangType> type(BaseInferer& base_inferer){
                try {
                    Inferer<DerivedExpr>& inferer = dynamic_cast<Inferer<DerivedExpr>&>(base_inferer);
                    return inferer.infer(static_cast<DerivedExpr*>(this));
                } catch (const std::bad_cast& e){
                    std::ostringstream err;
                    err << "Bad cast thrown in: " << typeid(DerivedExpr).name() << std::endl;
                    err << "Check if your Inferer implementation both inherits from Inferer<your expr>' and implements 'std::shared_ptr<LangType> infer(your expr*)'." << std::endl;
                    throw std::runtime_error(err.str());
                }
            }
    };

    class Assign: public ModuleStmt, public SimpleFuncStmt, public Visitable<Assign> {
        private:
            std::string varname_;
            Expr* expr_;

        public:
            Assign(std::string& name, Expr* expr): varname_(name), expr_(expr){}
            Assign(const char* name, Expr* expr): varname_(name), expr_(expr){}
            ~Assign(){
                delete expr_;
            }

            std::string varname() const { return varname_; }
            Expr* expr() const { return expr_; }
            std::string line() const {
                return varname_ + " = " + expr_->value_str();
            }
    };

    class IfStmt: public FuncStmt, public Visitable<IfStmt> {
        private:
            Expr* cond_;
            std::vector<FuncStmt*> body_;

        public:
            IfStmt(Expr* cond, std::vector<FuncStmt*>& body): cond_(cond), body_(body){}
            ~IfStmt(){
                for (FuncStmt* stmt : body_){
                    delete stmt;
                }
                delete cond_;
            }
            std::vector<std::string> lines() const override;

            Expr* cond() const { return cond_; }
            const std::vector<FuncStmt*> body() const { return body_; }
    };

    class ForLoop: public FuncStmt, public Visitable<ForLoop> {
        private:
            std::vector<Expr*> target_list_;
            Expr* container_;
            std::vector<FuncStmt*> body_;
        
        public:
            ForLoop(const std::vector<Expr*>& target_list, Expr* container,
                    const std::vector<FuncStmt*>& body):
                target_list_(target_list), container_(container), body_(body){}

            ~ForLoop(){
                for (Expr* target : target_list_){
                    delete target;
                }
                delete container_;
                for (FuncStmt* stmt : body_){
                    delete stmt;
                }
            }

            const std::vector<Expr*>& target_list() const { return target_list_; }
            Expr* container() const { return container_; }

            std::vector<std::string> lines() const override {
                std::vector<std::string> v;

                std::vector<std::string> arg_strs;
                for (Expr* target : target_list_){
                    arg_strs.push_back(target->value_str());
                }

                std::string line1 = "for " + join(arg_strs, ", ") + " in " + container_->value_str();
                v.push_back(line1);

                for (FuncStmt* stmt : body_){
                    for (std::string& stmt_line : stmt->lines()){
                        v.push_back(INDENT + stmt_line);
                    }
                }

                return v;
            }
    };

    class Call: public VisitableExpr<Call>, public Visitable<Call> {
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

    class Tuple: public VisitableExpr<Tuple>, public Visitable<Call> {
        private:
            std::vector<Expr*> contents_;

        public:
            Tuple(){}
            Tuple(const std::vector<Expr*>& contents): contents_(contents){}

            ~Tuple(){
                for (Expr* expr : contents_){
                    delete expr;
                }
            }

            const std::vector<Expr*> contents() const { return contents_; }

            std::string value_str() const override {
                std::vector<std::string> v;
                for (Expr* expr : contents_){
                    v.push_back(expr->value_str());
                }
                return join(v, ", ");
            }
    };

    class BinOperator: public virtual Node {
        public:
            virtual std::string symbol() const = 0;
            std::vector<std::string> lines() const {
                std::vector<std::string> v = {symbol()};
                return v;
            }
    };
    class Add: public BinOperator, public Visitable<Add> {
        public:
            std::string symbol() const { return "+"; }
    };
    class Sub: public BinOperator, public Visitable<Sub> {
        public:
            std::string symbol() const { return "-"; }
    };
    class Div: public BinOperator, public Visitable<Div> {
        public:
            std::string symbol() const { return "/"; }
    };
    class Mul: public BinOperator, public Visitable<Mul> {
        public:
            std::string symbol() const { return "*"; }
    };
    
    class Eq: public BinOperator, public Visitable<Eq> {
        public:
            std::string symbol() const { return "=="; }
    };
    class Ne: public BinOperator, public Visitable<Ne> {
        public:
            std::string symbol() const { return "!="; }
    };
    class Lt: public BinOperator, public Visitable<Lt> {
        public:
            std::string symbol() const { return "<"; }
    };
    class Gt: public BinOperator, public Visitable<Gt> {
        public:
            std::string symbol() const { return ">"; }
    };
    class Lte: public BinOperator, public Visitable<Lte> {
        public:
            std::string symbol() const { return "<="; }
    };
    class Gte: public BinOperator, public Visitable<Gte> {
        public:
            std::string symbol() const { return ">="; }
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

    class Int: public VisitableExpr<Int>, public Visitable<Int> {
        private:
            int value_;

        public:
            Int(const std::string&);
            Int(int);
            std::string value_str() const;
            int value() const { return value_; }
    };

    class NameExpr: public VisitableExpr<NameExpr>, public Visitable<NameExpr> {
        private:
            std::string name_;

        public:
            NameExpr(const std::string&);
            std::string value_str() const;
            std::string name() const { return name_; }
    };

    class String: public VisitableExpr<String>, public Visitable<String> {
        private:
            std::string value_;

        public:
            String(const std::string&);
            std::string value_str() const;
            std::string value() const { return value_; }
    };

    class BinExpr: public VisitableExpr<BinExpr>, public Visitable<BinExpr> {
        private:
            Expr* lhs_;
            BinOperator* op_;
            Expr* rhs_;

        public:
            BinExpr(Expr*, BinOperator*, Expr*);
            std::string value_str() const;
            ~BinExpr();

            Expr* lhs() const { return lhs_; }
            BinOperator* op() const { return op_; }
            Expr* rhs() const { return rhs_; }
    };

    class UnaryExpr: public VisitableExpr<UnaryExpr>, public Visitable<UnaryExpr> {
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

    // Variable arguments collector  
    class StarArgsTypeDecl: public TypeDecl, public Visitable<StarArgsTypeDecl> {
        public:
            std::string value_str() const override { return "*"; }
            std::shared_ptr<LangType> as_type() const override;
    };

    class StarArgsType: public LangType {
        public:
            TypeDecl* as_type_decl() const override { return new StarArgsTypeDecl; }
            bool equals(const LangType& other) const {
                const StarArgsType* other_star_args = dynamic_cast<const StarArgsType*>(&other);
                return other_star_args;
            }
    };

    class NameTypeDecl: public TypeDecl, public Visitable<NameTypeDecl> {
        private:
            std::string name_;

        public:
            NameTypeDecl(const std::string& name): name_(name){}
            NameTypeDecl(const char* name): name_(name){}
            std::string name() const { return name_; }
            std::string value_str() const override { return name_; }
            std::shared_ptr<LangType> as_type() const override;
    };

    class NameType: public LangType {
        private:
            std::string name_;

        public:
            NameType(const std::string& name): name_(name){}
            NameType(const char* name): name_(name){}

            std::string name() const { return name_; }

            TypeDecl* as_type_decl() const {
                NameTypeDecl* type_decl = new NameTypeDecl(name_);
                return type_decl;
            }

            bool equals(const LangType& other) const {
                const NameType* other_name = dynamic_cast<const NameType*>(&other);
                if (other_name){
                    return name_ == other_name->name();
                }
                else {
                    return false;
                }
            }
    };

    class FuncTypeDecl: public TypeDecl, public Visitable<FuncTypeDecl> {
        private:
            TypeDecl* return_type_;
            std::vector<TypeDecl*> args_;
            bool has_varargs_ = false;

        public:
            FuncTypeDecl(TypeDecl* return_type, 
                         const std::vector<TypeDecl*>& args,
                         bool has_varargs):
                return_type_(return_type), 
                args_(args),
                has_varargs_(has_varargs){}

            FuncTypeDecl(TypeDecl* return_type, 
                         const std::initializer_list<TypeDecl*>& args,
                         bool has_varargs):
                return_type_(return_type), 
                args_(args),
                has_varargs_(has_varargs){}

            std::shared_ptr<LangType> as_type() const override;

            ~FuncTypeDecl(){
                delete return_type_;
                for (TypeDecl* t : args_){
                    delete t;
                }
            }

            TypeDecl* return_type() const { return return_type_; }
            const std::vector<TypeDecl*>& args() const { return args_; }

            std::string value_str() const {
                std::string line = "(";

                std::vector<std::string> v;
                for (TypeDecl* decl : args_){
                    v.push_back(decl->value_str());
                }

                line += join(v, ", ") + ") -> " + return_type_->value_str();
                return line;
            }
    };

    class FuncType: public LangType {
        private:
            std::shared_ptr<LangType> return_type_;
            std::vector<std::shared_ptr<LangType>> args_;
            bool has_varargs_ = false;

        public:
            FuncType(std::shared_ptr<LangType> return_type, 
                     std::vector<std::shared_ptr<LangType>>& args,
                     bool has_varargs):
                return_type_(return_type), 
                args_(args),
                has_varargs_(has_varargs){}

            FuncType(std::shared_ptr<LangType> return_type, 
                     std::initializer_list<std::shared_ptr<LangType>> args,
                     bool has_varargs):
                return_type_(return_type), 
                args_(args),
                has_varargs_(has_varargs){}

            std::shared_ptr<LangType> return_type() const { return return_type_; }
            const std::vector<std::shared_ptr<LangType>>& args() const { return args_; }
            bool has_varargs() const { return has_varargs_; }

            TypeDecl* as_type_decl() const {
                std::vector<TypeDecl*> args;
                for (std::shared_ptr<LangType> arg : args_){
                    args.push_back(arg->as_type_decl());
                }

                return new FuncTypeDecl(return_type_->as_type_decl(), args,
                                        has_varargs_);
            }

            bool equals(const LangType& other) const {
                const FuncType* other_func = dynamic_cast<const FuncType*>(&other);

                if (!other_func){
                    return false;
                }

                // Check return type
                if (*return_type_ != *(other_func->return_type())){
                    return false;
                }

                // Check arguments  
                if (has_varargs_ != other_func->has_varargs()){
                    return false;
                }

                const std::vector<std::shared_ptr<LangType>>& other_args = other_func->args();
                if (args_.size() == other_args.size()){
                    for (std::size_t i = 0; i < args_.size(); ++i){
                        if (*(args_[i]) != *(other_args[i])){
                            return false;
                        }
                    }
                }

                return true;
            }
    };

    class VarDecl: public SimpleFuncStmt, public Visitable<VarDecl> {
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

            std::string line() const {
                return name_ + ": " + type_->value_str();
            }
    };

    class FuncArgs: public SimpleNode, public Visitable<FuncArgs> {
        private:
            std::vector<VarDecl*> pos_args_;
            std::vector<Assign*> keyword_args_;
            bool has_varargs_ = false;

        public:
            FuncArgs(){}

            FuncArgs(const std::vector<VarDecl*>& pos_args,
                     const std::vector<Assign*>& keyword_args,
                     bool has_varargs
                     ):
                pos_args_(pos_args),
                keyword_args_(keyword_args),
                has_varargs_(has_varargs){}

            FuncArgs(const std::initializer_list<VarDecl*>& pos_args,
                     const std::initializer_list<Assign*>& keyword_args,
                     bool has_varargs
                     ):
                pos_args_(pos_args),
                keyword_args_(keyword_args),
                has_varargs_(has_varargs){}

            ~FuncArgs(){
                for (VarDecl* arg : pos_args_){ delete arg; }
                for (Assign* arg : keyword_args_){ delete arg; }
            }

            const std::vector<VarDecl*>& pos_args() const { return pos_args_; }
            const std::vector<Assign*>& keyword_args() const { return keyword_args_; }
            bool has_varargs() const { return has_varargs_; }

            std::string line() const override;
    };

    class FuncDef: public ModuleStmt, public Visitable<FuncDef> {
        private:
            std::string func_name_;
            FuncArgs* args_;
            TypeDecl* return_type_decl_;
            std::vector<FuncStmt*> func_suite_;

        public:
            FuncDef(const std::string& func_name, 
                    FuncArgs* args,
                    TypeDecl* return_type_decl, 
                    std::vector<FuncStmt*>& func_suite):
                func_name_(func_name),
                args_(args),
                return_type_decl_(return_type_decl),
                func_suite_(func_suite){}

            std::vector<std::string> lines() const;

            ~FuncDef();

            const std::vector<FuncStmt*>& suite() const { return func_suite_; }
            std::string name() const { return func_name_; }
            TypeDecl* return_type_decl() const { return return_type_decl_; }
            FuncArgs* args() const { return args_; }
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
