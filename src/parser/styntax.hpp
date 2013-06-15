#pragma once

#include "parseval.hpp"

#include <vector>

namespace lambda{

        // i like c++ template magic, so this can simplify way of def syntax rules.
        //this 3 structs is used only in templates matching(like functional pattern matching)
        // wstring      => Token
        // id           => Ident
        // expr         => Expr
        // stmt         => Stmt
        // typeof{Token,Ident,Expr,Stmt} = ParseVal
        // compiler should inline all this functions, no overhead :)
        namespace Syntax{
                struct _SYNTAX_IDENTIFER_{};
                struct _SYNTAX_EXPRESSION{};
                struct _SYNTAX_STATEMENT{};
                struct _SYNTAX_STRING{};
                struct _SYNTAX_OPERATOR{};
                struct _SYNTAX_TERM{};
                struct _SYNTAX_NUMBER{};
                template<typename T> struct _SYNTAX_ANY{};
                _SYNTAX_IDENTIFER_              id;
                _SYNTAX_EXPRESSION              expr;
                _SYNTAX_STATEMENT               stmt;
                _SYNTAX_STRING                  str;
                _SYNTAX_OPERATOR                oper;
                _SYNTAX_NUMBER                  num;
                _SYNTAX_TERM                    term;

                inline ParseVal match(std::wstring s){
                        return ParseVal::Token(s);
                }
                inline ParseVal match(uint32_t id){
                        return ParseVal::Rule(id);
                }
                inline ParseVal match(_SYNTAX_IDENTIFER_){
                        return ParseVal::Ident();
                }
                inline ParseVal match(_SYNTAX_EXPRESSION){
                        return ParseVal::Expr();
                }
                inline ParseVal match(_SYNTAX_STATEMENT){
                        return ParseVal::Stmt();
                }
                inline ParseVal match(_SYNTAX_STRING){
                        return ParseVal::Str();
                }
                inline ParseVal match(_SYNTAX_OPERATOR){
                        return ParseVal::Oper();
                }
                inline ParseVal match(_SYNTAX_NUMBER){
                        return ParseVal::Num();
                }
                inline ParseVal match(_SYNTAX_TERM){
                        return ParseVal::Term();
                }
                inline ParseVal match(ParseVal val){
                        return val;
                }
                template<typename T> inline ParseVal match(_SYNTAX_ANY<T>){
                        return ParseVal::Any(match(T()));
                }
                template<typename T> inline _SYNTAX_ANY<T> any(T){return _SYNTAX_ANY<T>();}
        }
        // std::wstring tok - safes from some errors. syntax rule _must_ starts with uniq token.
        template<typename ... Rest>  inline std::vector<ParseVal> defSyntax(std::wstring tok ,Rest ... rest){
                return std::vector<ParseVal> {Syntax::match(tok),Syntax::match(rest)...};
        }
}
