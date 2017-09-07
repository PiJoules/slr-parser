#ifndef _COMPILER_H
#define _COMPILER_H

#include "lang.h"
#include "cpp_nodes.h"

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <cctype>
#include <algorithm>

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

            bool has_var(const std::string& varname) const {
                return varnames_.find(varname) != varnames_.end();
            }

            void check_var_exists(const std::string& varname) const {
                if (!has_var(varname)){
                    throw std::runtime_error("Unknown variable '" + varname + "'");
                }
            }

            std::shared_ptr<LangType> var_type(const std::string& varname) const { 
                if (!has_var(varname)){
                    throw std::runtime_error("Unknown variable '" + varname + "'");
                }
                else {
                    return varnames_.at(varname);
                }
            }

            const std::unordered_map<std::string, std::shared_ptr<LangType>>& varnames () const { return varnames_; }

            /**
             * Create a random variable name that does not exist yet in this scope.
             * Do this by adding an underscore to a random alpha numeric string whose 
             * initial size is that of the largest known varname.
             */
            std::string rand_varname() const {
                if (varnames_.empty()){
                    return "tmp_variable";
                }

                std::size_t max_varname_len = 0;

                for (auto it = varnames_.cbegin(); it != varnames_.cend(); ++it){
                    const std::string& existing = it->first;
                    max_varname_len = std::max(existing.size(), max_varname_len);
                }

                return "_" + rand_alphanum_str(max_varname_len);
            }
    };

    class Compiler: public parsing::Visitor<Module>,
                    public parsing::Visitor<FuncDef>,
                    public parsing::Visitor<ReturnStmt>,
                    public parsing::Visitor<VarDecl>,
                    public parsing::Visitor<Assign>,

                    public parsing::Visitor<ExprStmt>,
                    public parsing::Visitor<IfStmt>,
                    public parsing::Visitor<ForLoop>,

                    public parsing::Visitor<Call>,
                    public parsing::Visitor<BinExpr>,
                    public parsing::Visitor<String>,
                    public parsing::Visitor<NameExpr>,
                    public parsing::Visitor<Int>,
                    public parsing::Visitor<Tuple>,

                    public parsing::Visitor<Add>, 
                    public parsing::Visitor<Sub>,
                    public parsing::Visitor<Mul>,
                    public parsing::Visitor<Div>,

                    public parsing::Visitor<Eq>, 
                    public parsing::Visitor<Ne>,
                    public parsing::Visitor<Lt>,
                    public parsing::Visitor<Gt>,
                    public parsing::Visitor<Lte>,
                    public parsing::Visitor<Gte>,

                    public parsing::Visitor<NameTypeDecl>,
                    public parsing::Visitor<TupleTypeDecl>,
                    public parsing::Visitor<StringTypeDecl>,

                    // Inference 
                    public Inferer<Call>,
                    public Inferer<NameExpr>,
                    public Inferer<Tuple>,
                    public Inferer<String>
    {
        private:
            LangLexer lexer_;
            parsing::Parser parser_;

            std::unordered_map<std::string, LibData> include_libs_;

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

            std::shared_ptr<FuncType> funcdef_type(FuncDef&);

        public:
            using NodeVisitor::visit;
            using BaseInferer::infer;

            Compiler();
            std::shared_ptr<cppnodes::Module> compile(std::string);

            std::shared_ptr<void> visit(Module&);

            // Simple stmts
            std::shared_ptr<void> visit(ReturnStmt&);
            std::shared_ptr<void> visit(ExprStmt&);
            std::shared_ptr<void> visit(VarDecl&);
            std::shared_ptr<void> visit(Assign&);

            // Compound stmts
            std::shared_ptr<void> visit(FuncDef&);
            std::shared_ptr<void> visit(IfStmt&);
            std::shared_ptr<void> visit(ForLoop&);

            std::shared_ptr<void> visit(Call&);
            std::shared_ptr<void> visit(BinExpr&);
            std::shared_ptr<void> visit(Tuple&);

            // Atoms
            std::shared_ptr<void> visit(String&);
            std::shared_ptr<void> visit(NameExpr&);
            std::shared_ptr<void> visit(Int&);

            // Binary operators  
            std::shared_ptr<void> visit(Add&);
            std::shared_ptr<void> visit(Sub&);
            std::shared_ptr<void> visit(Mul&);
            std::shared_ptr<void> visit(Div&);

            std::shared_ptr<void> visit(Eq&);
            std::shared_ptr<void> visit(Ne&);
            std::shared_ptr<void> visit(Lt&);
            std::shared_ptr<void> visit(Gt&);
            std::shared_ptr<void> visit(Lte&);
            std::shared_ptr<void> visit(Gte&);

            std::shared_ptr<void> visit(NameTypeDecl&);
            std::shared_ptr<void> visit(TupleTypeDecl&);
            std::shared_ptr<void> visit(StringTypeDecl&);

            // Inference
            std::shared_ptr<LangType> infer(Call&);
            std::shared_ptr<LangType> infer(NameExpr&);
            std::shared_ptr<LangType> infer(Tuple&);
            std::shared_ptr<LangType> infer(String&);
    };

    // Language cmd interface 
    std::string compile_cpp_file(const std::string& src);
    std::string read_file(const std::string& filename);
    void write_file(const std::string& filename, const std::string& contents);
    std::string compile_lang_file(const std::string& src);
    void run_lang_file(const std::string& src);
}

#endif
