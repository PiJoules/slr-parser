#ifndef _COMPILER_H
#define _COMPILER_H

#include "lang.h"
#include "cpp_nodes.h"

namespace lang {
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
