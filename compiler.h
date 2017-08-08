#ifndef _COMPILER_H
#define _COMPILER_H

#include "lang.h"
#include "cpp_nodes.h"

#include <unordered_map>
#include <unordered_set>

namespace lang {
    // Mapping variable name to library name
    extern const std::unordered_map<std::string, std::string> LIB_VARIABLES;
    extern const std::unordered_set<std::string> LIB_SOURCES;

    class Compiler: public NodeVisitor,
                    public Visitor<Module>,
                    public Visitor<FuncDef>,
                    public Visitor<ReturnStmt>,
                    public Visitor<VarDecl>,

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
                    public Visitor<Gte>
    {
        private:
            LangLexer lexer_;
            parsing::Parser parser_;

            std::unordered_set<std::string> include_libs_;

        public:
            Compiler();
            cppnodes::Module* compile(std::string);

            void* visit(Module*);

            // Simple stmts
            void* visit(ReturnStmt*);
            void* visit(ExprStmt*);
            void* visit(VarDecl*);

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
    };

    // Language cmd interface 
    std::string compile_cpp_file(const std::string& src);
    std::string read_file(const std::string& filename);
    void write_file(const std::string& filename, const std::string& contents);
    std::string compile_lang_file(const std::string& src);
    void run_lang_file(const std::string& src);
}

#endif
