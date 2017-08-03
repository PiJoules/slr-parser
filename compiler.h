#ifndef _COMPILER_H
#define _COMPILER_H

#include "lang.h"
#include "cpp_nodes.h"

namespace lang {
    class Compiler: public NodeVisitor,
                    public Visitor<Module>,
                    //public Visitor<ModuleStmt>,
                    public Visitor<FuncDef>,
                    public Visitor<ReturnStmt>,
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
            void* visit(Int*);
    };
}

#endif
