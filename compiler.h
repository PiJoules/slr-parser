#ifndef _COMPILER_H
#define _COMPILER_H

#include "lang.h"
#include "cpp_nodes.h"

namespace lang {
    class Compiler {
        private:
            LangLexer lexer_;
            parsing::Parser parser_;

        public:
            Compiler();
            cppnodes::Module* compile(std::string);

            cppnodes::Module* visit_module(lang::Module*);
            cppnodes::FuncDef* visit_funcdef(lang::FuncDef*);
            cppnodes::ReturnStmt* visit_return(lang::ReturnStmt*);
            cppnodes::Int visit_int(lang::Int);
    };
}

#endif
