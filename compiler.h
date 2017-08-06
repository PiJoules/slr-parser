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
                    public Visitor<ExprStmt>,
                    public Visitor<Call>,
                    public Visitor<String>,
                    public Visitor<NameExpr>,
                    public Visitor<Int>
    {
        private:
            LangLexer lexer_;
            parsing::Parser parser_;

            std::unordered_set<std::string> include_libs_;

        public:
            Compiler();
            cppnodes::Module* compile(std::string);

            void* visit(Module*);

            void* visit(FuncDef*);

            void* visit(ReturnStmt*);
            void* visit(ExprStmt*);

            void* visit(Call*);
            void* visit(String*);
            void* visit(NameExpr*);
            void* visit(Int*);
    };
}

#endif
