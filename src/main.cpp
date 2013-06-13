#include "lexer/lexer.hpp"
#include <iostream>
#include "utils/debug.hpp"
#include "parser/parser.hpp"
using namespace lambda;
int main(int argc, char const *argv[])
{
        Parser par;

        // i'll add value-blocks later. so '{...}' would be <expression> too.

        using namespace Syntax;

        // <if> ::= 'if' <expression> '{' <expression> '}' 'else' '{' <expression> '}'
        par.defExpr(L"if-then-else",
                defSyntax(
                        (L"if"), expr,
                                (L"{"), expr, (L"}"),
                        (L"else"),
                                (L"{"), expr, (L"}")
                ), 
                [](Parser::ParseInfo & info ){
                        return new Expression();         
                }
        );
        // <when> ::= 'when' <expression> '{' <statement> '}' 
        par.defStmt(L"when",
                defSyntax(
                        (L"when"), expr,
                                (L"{"), stmt, (L"}")
                ), 
                [](Parser::ParseInfo & info ){
                        std::wcerr << L"when alloc\n";
                        return new Statement();         
                }
        );
        // like <when>, but condition must be false.
        // <unless> ::= 'when' <expression> '{' <statement> '}' 
        par.defStmt(L"unless",
                defSyntax(
                        (L"unless"), expr,
                                (L"{"), stmt, (L"}")
                ), 
                [](Parser::ParseInfo & info ){
                        return new Statement();         
                }
        );

        // new syntax :)
        // <let> ::= 'let' <identifer> '=' <expression>
        auto let = par.defStmt(L"let",
                defSyntax(L"let",id,L"=",expr),
                [](Parser::ParseInfo&){
                        std::wcerr << L"let alloc\n";
                        return new Statement();
                }
        );
        // now looks like BNF 
        // <def> ::= 'def' <identifer> '=' <expression>
        par.defStmt(L"def",
                defSyntax(L"def",id,L"=",expr),
                [](Parser::ParseInfo&){
                        return new Statement();
                }
        );
        // <block> ::= '{' <statement>* '}'
        par.defStmt(L"block",ParseVal::Token(L"{"), [](Parser& par,Lexer& lex){
                auto end = lex.TryRecognize(L"}");
                while(lex.nextTok().id!=end){
                        par.expectStatement();
                }
                std::wcerr << L"block alloc\n";
                return new Statement();
        });
        // <for> ::= 'for' <let> ('let' <identifer> '=' <expression>) 'to' <expression> 'do' <statement>
        par.defStmt(L"for", defSyntax(L"for",let,L"to",expr,L"do",stmt),
                [](Parser::ParseInfo&){std::wcerr<<L"for alloc\n";return new Statement();});
        par.addData(L"for let i = 1 to 10 do { when a b let c = 10 }", L"test");
        par.Parse();
        par.showRules();
        par.showError();
        return 0;
}