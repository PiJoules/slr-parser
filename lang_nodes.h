#ifndef _LANG_NODES_H
#define _LANG_NODES_H 

#include <vector>
#include <string>

#include <typeinfo>
#include <stdexcept>
#include <sstream>
#include <initializer_list>
#include <memory>
#include <iostream>

#include "parser.h"

const std::string INDENT = "    ";

namespace lang {
    class ModuleStmt: public virtual parsing::Node {};
    class FuncStmt: public virtual parsing::Node {};

    class SimpleFuncStmt: public virtual FuncStmt, public virtual parsing::SimpleNode {};

    class LangType;

    // The type of an object
    class TypeDecl: public virtual parsing::SimpleNode {
        public:
            virtual std::shared_ptr<LangType> as_type() const = 0;
    };

    /*********** Type Inference *******/

    class LangType;
    class Expr;

    class BaseInferer {
        public:
            virtual ~BaseInferer(){}
            std::shared_ptr<LangType> infer(Expr&);
    };

    class LangType {
        public:
            virtual std::shared_ptr<TypeDecl> as_type_decl() const = 0;
            virtual bool equals(const LangType&) const = 0;

            bool operator==(const LangType& other) const { return equals(other); }
            bool operator!=(const LangType& other) const { return !(*this == other); }
    };

    class Expr: public virtual parsing::SimpleNode {
        public:
            // The string representation of the value this expression holds
            virtual std::shared_ptr<LangType> type(BaseInferer&) = 0;
    };

    template <typename VisitedExpr>
    class Inferer: public virtual BaseInferer {
        public:
            virtual std::shared_ptr<LangType> infer(VisitedExpr&) = 0;
    };

    template <typename DerivedExpr>
    class VisitableExpr: public virtual Expr {
        public:
            std::shared_ptr<LangType> type(BaseInferer& base_inferer){
                try {
                    Inferer<DerivedExpr>& inferer = dynamic_cast<Inferer<DerivedExpr>&>(base_inferer);
                    return inferer.infer(static_cast<DerivedExpr&>(*this));
                } catch (const std::bad_cast& e){
                    std::ostringstream err;
                    err << "Bad cast thrown in: " << typeid(DerivedExpr).name() << std::endl;
                    err << "Check if your Inferer implementation both inherits from Inferer<your expr>' and implements 'std::shared_ptr<LangType> infer(your expr&)'." << std::endl;
                    throw std::runtime_error(err.str());
                }
            }
    };

    /*********** End Type Inference *******/

    class Assign: public ModuleStmt, public SimpleFuncStmt, public parsing::Visitable<Assign> {
        private:
            std::string varname_;
            std::shared_ptr<Expr> expr_;

        public:
            Assign(const std::string& name, std::shared_ptr<Expr> expr): varname_(name), expr_(expr){}
            Assign(const char* name, std::shared_ptr<Expr> expr): varname_(name), expr_(expr){}

            std::string varname() const { return varname_; }
            std::shared_ptr<Expr> expr() const { return expr_; }
            std::string line() const {
                return varname_ + " = " + expr_->line();
            }
    };

    class IfStmt: public FuncStmt, public parsing::Visitable<IfStmt> {
        private:
            std::shared_ptr<Expr> cond_;
            std::vector<std::shared_ptr<FuncStmt>> body_;

        public:
            IfStmt(std::shared_ptr<Expr> cond, 
                   const std::vector<std::shared_ptr<FuncStmt>>& body): cond_(cond), body_(body){}
            std::vector<std::string> lines() const override;

            std::shared_ptr<Expr> cond() const { return cond_; }
            const std::vector<std::shared_ptr<FuncStmt>> body() const { return body_; }
    };

    class TargetList: public parsing::Visitable<TargetList> {
    
    };

    class ForLoop: public FuncStmt, public parsing::Visitable<ForLoop> {
        private:
            std::vector<std::shared_ptr<Expr>> target_list_;
            std::shared_ptr<Expr> container_;
            std::vector<std::shared_ptr<FuncStmt>> body_;
        
        public:
            ForLoop(const std::vector<std::shared_ptr<Expr>>& target_list, 
                    std::shared_ptr<Expr> container,
                    const std::vector<std::shared_ptr<FuncStmt>>& body):
                target_list_(target_list), container_(container), body_(body){}

            const std::vector<std::shared_ptr<Expr>>& target_list() const { return target_list_; }
            std::shared_ptr<Expr> container() const { return container_; }
            const std::vector<std::shared_ptr<FuncStmt>>& body() const { return body_; }

            std::vector<std::string> lines() const override {
                std::vector<std::string> v;

                std::vector<std::string> arg_strs;
                for (std::shared_ptr<Expr> target : target_list_){
                    arg_strs.push_back(target->line());
                }

                std::string line1 = "for " + join(arg_strs, ", ") + " in " + container_->line();
                v.push_back(line1);

                for (std::shared_ptr<FuncStmt> stmt : body_){
                    for (std::string& stmt_line : stmt->lines()){
                        v.push_back(INDENT + stmt_line);
                    }
                }

                return v;
            }
    };

    class Call: public VisitableExpr<Call>, public parsing::Visitable<Call> {
        private:
            std::shared_ptr<Expr> func_;
            std::vector<std::shared_ptr<Expr>> args_;

        public:
            Call(std::shared_ptr<Expr> func): func_(func){}
            Call(std::shared_ptr<Expr> func, const std::vector<std::shared_ptr<Expr>>& args):
                func_(func), args_(args){}

            std::shared_ptr<Expr> func() const { return func_; }
            const std::vector<std::shared_ptr<Expr>>& args() const { return args_; }

            std::string line() const override;
    };

    class MemberAccess: public VisitableExpr<MemberAccess>, public parsing::Visitable<MemberAccess> {
        private:
            std::shared_ptr<Expr> base_;
            std::string member_;

        public:
            MemberAccess(std::shared_ptr<Expr> base, 
                         const std::string& member): 
                base_(base), member_(member){}

            std::shared_ptr<Expr> base() const { return base_; }
            std::string member() const { return member_; }

            std::string line() const override {
                return base_->line() + "." + member_;
            }
    };

    class Tuple: public VisitableExpr<Tuple>, public parsing::Visitable<Tuple> {
        private:
            std::vector<std::shared_ptr<Expr>> contents_;

        public:
            Tuple(){}
            Tuple(const std::vector<std::shared_ptr<Expr>>& contents): contents_(contents){}

            const std::vector<std::shared_ptr<Expr>> contents() const { return contents_; }

            std::string line() const override {
                std::vector<std::string> v;
                for (std::shared_ptr<Expr> expr : contents_){
                    v.push_back(expr->line());
                }
                return join(v, ", ");
            }
    };

    class TupleTypeDecl: public TypeDecl, public parsing::Visitable<TupleTypeDecl> {
        private:
            std::vector<std::shared_ptr<TypeDecl>> contents_;

        public:
            TupleTypeDecl(){}
            TupleTypeDecl(const std::vector<std::shared_ptr<TypeDecl>>& contents): contents_(contents){}

            std::string line() const override {
                std::vector<std::string> v;
                for (std::shared_ptr<TypeDecl> decl : contents_){
                    v.push_back(decl->line());
                }
                return "tuple[" + join(v, ",") + "]";
            }

            std::shared_ptr<LangType> as_type() const override;

            const std::vector<std::shared_ptr<TypeDecl>>& contents() const { return contents_; }
    };

    class TupleType: public LangType {
        private:
            std::vector<std::shared_ptr<LangType>> contents_;

        public:
            TupleType(){}
            TupleType(const std::vector<std::shared_ptr<LangType>>& contents): contents_(contents){}

            const std::vector<std::shared_ptr<LangType>>& contents() const { return contents_; }

            std::shared_ptr<TypeDecl> as_type_decl() const {
                std::vector<std::shared_ptr<TypeDecl>> content_type_decls;

                for (auto& lang_type : contents_){
                    content_type_decls.push_back(lang_type->as_type_decl());
                }

                return std::make_shared<TupleTypeDecl>(content_type_decls);
            }

            bool equals(const LangType& other) const {
                const TupleType* other_tuple = dynamic_cast<const TupleType*>(&other);
                if (other_tuple){
                    const auto& other_contents = other_tuple->contents();
                    if (contents_.size() == other_contents.size()){
                        for (std::size_t i = 0; i < contents_.size(); ++i){
                            if (*(contents_[i]) != *(other_contents[i])){
                                return false;
                            }
                        }
                        return true;
                    }
                }
                return false;
            }
    };

    class BinOperator: public parsing::SimpleNode {
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

    // TODO: Maybe we can merge this and the binary operator class
    // if they don't really end up having different logic in the long run.
    class UnaryOperator: public parsing::SimpleNode {
        public:
            virtual std::string symbol() const = 0;
            std::string line() const override { return symbol(); }
    };
    class USub: public UnaryOperator, public parsing::Visitable<USub> {
        public:
            std::string symbol() const { return "-"; }
    };

    class Int: public VisitableExpr<Int>, public parsing::Visitable<Int> {
        private:
            int value_;

        public:
            Int(const std::string&);
            Int(int);
            std::string line() const;
            int value() const { return value_; }
    };

    class NameExpr: public VisitableExpr<NameExpr>, public parsing::Visitable<NameExpr> {
        private:
            std::string name_;

        public:
            NameExpr(const std::string&);
            std::string line() const;
            std::string name() const { return name_; }
    };

    class String: public VisitableExpr<String>, public parsing::Visitable<String> {
        private:
            std::string value_;

        public:
            String(const std::string&);
            std::string line() const;
            std::string value() const { return value_; }
    };

    class StringTypeDecl: public TypeDecl, public parsing::Visitable<StringTypeDecl> {
        public:
            std::string line() const override { return "str"; }
            std::shared_ptr<LangType> as_type() const override;
    };

    class StringType: public LangType {
        public:
            std::shared_ptr<TypeDecl> as_type_decl() const {
                return std::make_shared<StringTypeDecl>();
            }

            bool equals(const LangType& other) const {
                const StringType* other_str = dynamic_cast<const StringType*>(&other);
                return other_str;
            }
    };

    class BinExpr: public VisitableExpr<BinExpr>, public parsing::Visitable<BinExpr> {
        private:
            std::shared_ptr<Expr> lhs_;
            std::shared_ptr<BinOperator> op_;
            std::shared_ptr<Expr> rhs_;

        public:
            BinExpr(std::shared_ptr<Expr> lhs, std::shared_ptr<BinOperator> op, 
                    std::shared_ptr<Expr> rhs):
                lhs_(lhs), op_(op), rhs_(rhs){}
            std::string line() const override;

            std::shared_ptr<Expr> lhs() const { return lhs_; }
            std::shared_ptr<BinOperator> op() const { return op_; }
            std::shared_ptr<Expr> rhs() const { return rhs_; }
    };

    class UnaryExpr: public VisitableExpr<UnaryExpr>, public parsing::Visitable<UnaryExpr> {
        private: 
            std::shared_ptr<Expr> expr_;
            std::shared_ptr<UnaryOperator> op_;

        public:
            UnaryExpr(std::shared_ptr<Expr> expr, std::shared_ptr<UnaryOperator> op):
                expr_(expr), op_(op){}
            std::string line() const override;
    };

    class ExprStmt: public SimpleFuncStmt, public parsing::Visitable<ExprStmt> {
        private:
            std::shared_ptr<Expr> expr_;

        public:
            ExprStmt(std::shared_ptr<Expr> expr): expr_(expr){}
            std::string line() const override { return expr_->line(); }
            std::shared_ptr<Expr> expr() const { return expr_; }
    };

    class ReturnStmt: public SimpleFuncStmt, public parsing::Visitable<ReturnStmt> {
        private:
            std::shared_ptr<Expr> expr_;

        public:
            ReturnStmt(std::shared_ptr<Expr> expr): expr_(expr){}
            std::string line() const override { return "return " + expr_->line(); }

            std::shared_ptr<Expr> expr() const { return expr_; }
    };

    // Variable arguments collector  
    class StarArgsTypeDecl: public TypeDecl, public parsing::Visitable<StarArgsTypeDecl> {
        public:
            std::string line() const override { return "*"; }
            std::shared_ptr<LangType> as_type() const override;
    };

    class StarArgsType: public LangType {
        public:
            std::shared_ptr<TypeDecl> as_type_decl() const override { 
                return std::make_shared<StarArgsTypeDecl>(); 
            }
            bool equals(const LangType& other) const {
                const StarArgsType* other_star_args = dynamic_cast<const StarArgsType*>(&other);
                return other_star_args;
            }
    };

    class NameTypeDecl: public TypeDecl, public parsing::Visitable<NameTypeDecl> {
        private:
            std::string name_;

        public:
            NameTypeDecl(const std::string& name): name_(name){}
            NameTypeDecl(const char* name): name_(name){}
            std::string name() const { return name_; }
            std::string line() const override { return name_; }
            std::shared_ptr<LangType> as_type() const override;
    };

    class NameType: public LangType {
        private:
            std::string name_;

        public:
            NameType(const std::string& name): name_(name){}
            NameType(const char* name): name_(name){}

            std::string name() const { return name_; }

            std::shared_ptr<TypeDecl> as_type_decl() const {
                return std::make_shared<NameTypeDecl>(name_);
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

    class FuncTypeDecl: public TypeDecl, public parsing::Visitable<FuncTypeDecl> {
        private:
            std::shared_ptr<TypeDecl> return_type_;
            std::vector<std::shared_ptr<TypeDecl>> args_;
            bool has_varargs_ = false;

        public:
            FuncTypeDecl(std::shared_ptr<TypeDecl> return_type, 
                         const std::vector<std::shared_ptr<TypeDecl>>& args,
                         bool has_varargs):
                return_type_(return_type), 
                args_(args),
                has_varargs_(has_varargs){}

            FuncTypeDecl(std::shared_ptr<TypeDecl> return_type, 
                         const std::initializer_list<std::shared_ptr<TypeDecl>>& args,
                         bool has_varargs):
                return_type_(return_type), 
                args_(args),
                has_varargs_(has_varargs){}

            std::shared_ptr<LangType> as_type() const override;

            std::shared_ptr<TypeDecl> return_type() const { return return_type_; }
            const std::vector<std::shared_ptr<TypeDecl>>& args() const { return args_; }

            std::string line() const override {
                std::string line = "(";

                std::vector<std::string> v;
                for (std::shared_ptr<TypeDecl> decl : args_){
                    v.push_back(decl->line());
                }

                line += join(v, ", ") + ") -> " + return_type_->line();
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

            std::shared_ptr<TypeDecl> as_type_decl() const {
                std::vector<std::shared_ptr<TypeDecl>> args;
                for (std::shared_ptr<LangType> arg : args_){
                    args.push_back(arg->as_type_decl());
                }

                return std::make_shared<FuncTypeDecl>(return_type_->as_type_decl(), args,
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

    class VarDecl: public SimpleFuncStmt, public parsing::Visitable<VarDecl> {
        private:
            std::string name_;
            std::shared_ptr<TypeDecl> type_;

        public:
            VarDecl(std::string& name, std::shared_ptr<TypeDecl> type): name_(name), type_(type){}

            std::string name() const { return name_; }
            std::shared_ptr<TypeDecl> type() const { return type_; }

            std::string line() const override {
                return name_ + ": " + type_->line();
            }
    };

    class FuncArgs: public parsing::SimpleNode, public parsing::Visitable<FuncArgs> {
        private:
            std::vector<std::shared_ptr<VarDecl>> pos_args_;
            std::vector<std::shared_ptr<Assign>> keyword_args_;
            bool has_varargs_ = false;

        public:
            FuncArgs(){}

            FuncArgs(const std::vector<std::shared_ptr<VarDecl>>& pos_args,
                     const std::vector<std::shared_ptr<Assign>>& keyword_args,
                     bool has_varargs
                     ):
                pos_args_(pos_args),
                keyword_args_(keyword_args),
                has_varargs_(has_varargs){}

            FuncArgs(const std::initializer_list<std::shared_ptr<VarDecl>>& pos_args,
                     const std::initializer_list<std::shared_ptr<Assign>>& keyword_args,
                     bool has_varargs
                     ):
                pos_args_(pos_args),
                keyword_args_(keyword_args),
                has_varargs_(has_varargs){}

            const std::vector<std::shared_ptr<VarDecl>>& pos_args() const { return pos_args_; }
            const std::vector<std::shared_ptr<Assign>>& keyword_args() const { return keyword_args_; }
            bool has_varargs() const { return has_varargs_; }

            std::string line() const override;
    };

    class FuncDef: public ModuleStmt, public parsing::Visitable<FuncDef> {
        private:
            std::string func_name_;
            std::shared_ptr<FuncArgs> args_;
            std::shared_ptr<TypeDecl> return_type_decl_;
            std::vector<std::shared_ptr<FuncStmt>> func_suite_;

        public:
            FuncDef(const std::string& func_name, 
                    std::shared_ptr<FuncArgs> args,
                    std::shared_ptr<TypeDecl> return_type_decl, 
                    std::vector<std::shared_ptr<FuncStmt>>& func_suite):
                func_name_(func_name),
                args_(args),
                return_type_decl_(return_type_decl),
                func_suite_(func_suite){}

            std::vector<std::string> lines() const;

            const std::vector<std::shared_ptr<FuncStmt>>& suite() const { return func_suite_; }
            std::string name() const { return func_name_; }
            std::shared_ptr<TypeDecl> return_type_decl() const { return return_type_decl_; }
            std::shared_ptr<FuncArgs> args() const { return args_; }
    };

    class Module: public parsing::Visitable<Module> {
        private:
            std::vector<std::shared_ptr<ModuleStmt>> body_;

        public:
            Module(std::vector<std::shared_ptr<ModuleStmt>>& body): body_(body){}
            const std::vector<std::shared_ptr<ModuleStmt>>& body() const { return body_; }
            std::vector<std::string> lines() const override;
    };
}

#endif
