#ifndef _CPP_NODES_H
#define _CPP_NODES_H

#include "parser.h"

namespace cppnodes {
    // Base node representing a whole .c file
    class Module: public parsing::Node {
        private:
            std::vector<parsing::Node*> body_;

        public:
            Module(const std::vector<parsing::Node*>&);
            std::vector<std::string> lines() const;
            ~Module();
    };

    // Base Expression node
    class Expr: public parsing::Node {
        public:
            // All expressions can be written on one line
            virtual std::string value_str() const = 0;
            std::vector<std::string> lines() const;
    };

    class Stmt: public parsing::Node {};

    // One line statement
    class SimpleStmt: public Stmt {
        public:
            virtual std::string value_str() const = 0;
            std::vector<std::string> lines() const;
    };

    // Multi line statement
    class CompoundStmt: public Stmt {};

    // Variable declaration
    class VarDecl: public parsing::Node {};

    // int x;
    class RegVarDecl: public VarDecl {
        private:
            std::string name_;
            std::string type_;

        public:
            RegVarDecl(const char* name, const char* type);
            RegVarDecl(std::string& name, std::string& type);
            std::vector<std::string> lines() const;
    };

    // int (*func)(int arg1, int arg2)
    class FuncDecl: public VarDecl {};

    // struct Person person;
    class StructDecl: public VarDecl {};

    // enum Color color;
    class EnumDecl: public VarDecl {};

    class FuncDef: public CompoundStmt {
        private:
            std::string name_;
            std::string type_;
            std::vector<VarDecl*> args_;
            std::vector<parsing::Node*> body_;

        public:
            FuncDef(const std::string&, const std::string&, 
                    const std::vector<VarDecl*>&,
                    const std::vector<parsing::Node*>&);
            ~FuncDef();
            std::vector<std::string> lines() const;
    };

    class Name: public Expr {
        private:
            std::string id_;

        public:
            Name(const char*);
            Name(std::string&);
            std::string value_str() const;
    };

    class Call: public Expr {
        private:
            Expr* func_;
            std::vector<Expr*> args_;

        public:
            Call(Expr*, std::vector<Expr*>&);
            ~Call();
            std::string value_str() const;
    };

    class ReturnStmt: public SimpleStmt {
        private:
            Expr* expr_;

        public:
            ReturnStmt(Expr* expr);
            ~ReturnStmt();
            std::string value_str() const;
    };

    /**
     * Macros
     */ 
    class Macro: public parsing::Node {};

    // Single line macro
    class SimpleMacro: Macro {
        public:
            virtual std::string value_str() const = 0;
            std::vector<std::string> lines() const;
    };

    // Define without the second argument
    class SimpleDefine: public SimpleMacro {
        private:
            std::string name_;

        public:
            SimpleDefine(std::string&);
            std::string value_str() const;
    };

    class Ifndef: public SimpleMacro {
        private:
            std::string name_;

        public:
            Ifndef(std::string&);
            std::string value_str() const;
    };

    class Endif: public SimpleMacro {
        public:
            Endif();
            std::string value_str() const;
    };
}

#endif
