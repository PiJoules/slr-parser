#ifndef _COMPILER_H
#define _COMPILER_H

#include "lang.h"
#include "cpp_nodes.h"

#include <unordered_map>
#include <unordered_set>

namespace lang {
    // Mapping variable name to library 
    typedef struct LibData LibData;
    struct LibData {
        std::string lib_filename;
        std::unordered_map<std::string, TypeDecl*> lib_var_types;
    };
    extern const std::unordered_map<std::string, LibData> LIB_DATA;

    class Scope {
        private:
            std::unordered_map<std::string, TypeDecl*> varnames_;

        public:
            Scope(){}
            Scope(const Scope& scope){
                varnames_ = scope.varnames();
            }

            void add_var(const std::string& varname, TypeDecl* type){
                assert(varnames_.find(varname) == varnames_.end());
                varnames_[varname] = type;
            }
            void check_var_exists(const std::string& varname) const {
                auto found = varnames_.find(varname);
                if (found == varnames_.end()){
                    throw std::runtime_error("Unknown variable '" + varname + "'");
                }
            }
            TypeDecl* var_type(const std::string& varname) const { 
                auto found = varnames_.find(varname);
                if (found == varnames_.end()){
                    throw std::runtime_error("Unknown variable '" + varname + "'");
                }
                else {
                    return found->second;
                }
            }
            const std::unordered_map<std::string, TypeDecl*>& varnames () const { return varnames_; }
    };

    class Compiler: public NodeVisitor,
                    public Visitor<Module>,
                    public Visitor<FuncDef>,
                    public Visitor<ReturnStmt>,
                    public Visitor<VarDecl>,
                    public Visitor<Assign>,

                    public Visitor<ExprStmt>,
                    public Visitor<IfStmt>,

                    public Visitor<Call>,
                    public Visitor<BinExpr>,
                    public Visitor<String>,
                    public Visitor<NameExpr>,
                    public Visitor<Int>,

                    public Visitor<Add>, 
                    public Visitor<Sub>,
                    public Visitor<Mul>,
                    public Visitor<Div>,

                    public Visitor<Eq>, 
                    public Visitor<Ne>,
                    public Visitor<Lt>,
                    public Visitor<Gt>,
                    public Visitor<Lte>,
                    public Visitor<Gte>,

                    public Visitor<NameTypeDecl>,

                    // Inference
                    public BaseInferer,
                    public Inferer<Call>,
                    public Inferer<NameExpr>
    {
        private:
            LangLexer lexer_;
            parsing::Parser parser_;

            std::unordered_set<std::string> include_libs_;
            std::string cached_type_name_;

            void import_builtin_lib(const LibData& lib);  // Done to global scope 

            // Scope stack 
            std::vector<Scope> scope_stack_ = {};
            Scope& global_scope() { return scope_stack_.front(); }
            Scope& current_scope() { return scope_stack_.back(); }
            void enter_scope(){ 
                Scope new_scope = scope_stack_.back();
                scope_stack_.push_back(new_scope); 
            }
            void exit_scope(){ scope_stack_.pop_back(); }

            FuncTypeDecl* funcdef_type(FuncDef*) const;

        public:
            Compiler();
            cppnodes::Module* compile(std::string);

            TypeDecl* infer(Expr* expr){
                return expr->type(*this);
            }

            void* visit(Module*);

            // Simple stmts
            void* visit(ReturnStmt*);
            void* visit(ExprStmt*);
            void* visit(VarDecl*);
            void* visit(Assign*);

            // Compound stmts
            void* visit(FuncDef*);
            void* visit(IfStmt*);

            void* visit(Call*);
            void* visit(BinExpr*);

            // Atoms
            void* visit(String*);
            void* visit(NameExpr*);
            void* visit(Int*);

            // Binary operators  
            void* visit(Add*);
            void* visit(Sub*);
            void* visit(Mul*);
            void* visit(Div*);

            void* visit(Eq*);
            void* visit(Ne*);
            void* visit(Lt*);
            void* visit(Gt*);
            void* visit(Lte*);
            void* visit(Gte*);

            void* visit(NameTypeDecl*);

            // Inference
            TypeDecl* infer(Call*);
            TypeDecl* infer(NameExpr*);
    };

    // Language cmd interface 
    std::string compile_cpp_file(const std::string& src);
    std::string read_file(const std::string& filename);
    void write_file(const std::string& filename, const std::string& contents);
    std::string compile_lang_file(const std::string& src);
    void run_lang_file(const std::string& src);
}

#endif
