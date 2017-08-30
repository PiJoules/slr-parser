#ifndef _COMPILER_H
#define _COMPILER_H

#include "lang.h"
#include "cpp_nodes.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace lang {
    // Mapping variable name to library 
    typedef struct LibData LibData;
    struct LibData {
        std::string lib_filename;
        std::unordered_map<std::string, std::shared_ptr<LangType>> lib_var_types;
    };

    LibData create_io_lib();

    /**
     * NOTE: The scope only holds pointers to TypeDecls created outside of it,
     * so these pointers should be free'd outside of this scope.
     */
    class Scope {
        private:
            std::unordered_map<std::string, std::shared_ptr<LangType>> varnames_;

        public:
            Scope(){}
            Scope(const Scope& scope): varnames_(scope.varnames()){}

            /**
             * TypeDecls can be added to the scope through:
             * - Importing the builtin libs at start 
             * - Creation of a new type (func/class defs)
             */
            void add_var(const std::string& varname, std::shared_ptr<LangType> type){
                auto found_var = varnames_.find(varname);
                if (found_var == varnames_.end()){
                    varnames_[varname] = type;
                }
                else {
                    // Check types match
                    assert(*(found_var->second.get()) == *(type.get()));
                }
            }

            bool var_exists(const std::string& varname) const {
                return varnames_.find(varname) != varnames_.end();
            }

            void check_var_exists(const std::string& varname) const {
                if (!var_exists(varname)){
                    throw std::runtime_error("Unknown variable '" + varname + "'");
                }
            }

            std::shared_ptr<LangType> var_type(const std::string& varname) const { 
                if (!var_exists(varname)){
                    throw std::runtime_error("Unknown variable '" + varname + "'");
                }
                else {
                    return varnames_.at(varname);
                }
            }
            //const std::unordered_map<std::string, std::shared_ptr<LangType>>& varnames () const { return varnames_; }
            std::unordered_map<std::string, std::shared_ptr<LangType>> varnames () const { 
                std::unordered_map<std::string, std::shared_ptr<LangType>> varnames_cpy;
                for (auto it = varnames_.begin(); it != varnames_.end(); ++it){
                    varnames_cpy[it->first] = it->second;
                }
                return varnames_cpy; 
            }
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
                    public Visitor<TupleTypeDecl>,

                    // Inference
                    public BaseInferer,
                    public Inferer<Call>,
                    public Inferer<NameExpr>,
                    public Inferer<Tuple>,
                    public Inferer<String>
    {
        private:
            LangLexer lexer_;
            parsing::Parser parser_;

            std::unordered_map<std::string, LibData> include_libs_;
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

            std::shared_ptr<FuncType> funcdef_type(FuncDef*);

        public:
            Compiler();
            ~Compiler();
            cppnodes::Module* compile(std::string);

            std::shared_ptr<LangType> infer(Expr* expr){
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
            void* visit(TupleTypeDecl*);

            // Inference
            std::shared_ptr<LangType> infer(Call*);
            std::shared_ptr<LangType> infer(NameExpr*);
            std::shared_ptr<LangType> infer(Tuple*);
            std::shared_ptr<LangType> infer(String*);
    };

    // Language cmd interface 
    std::string compile_cpp_file(const std::string& src);
    std::string read_file(const std::string& filename);
    void write_file(const std::string& filename, const std::string& contents);
    std::string compile_lang_file(const std::string& src);
    void run_lang_file(const std::string& src);
}

#endif
