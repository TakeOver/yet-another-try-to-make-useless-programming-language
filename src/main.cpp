#include <iostream>

#include "lexer/lexer.hpp"
#include "utils/debug.hpp"
#include "parser/parser.hpp"
#include "parser/syntax.hpp"


int main(int argc, char const *argv[])
{

        // i'll add value-blocks later. so '{...}' would be <expression> too.
        using namespace lambda;
        using namespace Syntax;
        
        Parser par;

        // <block> ::= '{' <statement>* '}'
        auto block = par.defStmt(L"block",
                defSyntax(
                        (L"{"), any(stmt), (L"}")
                ),
                [](Parser::ParseInfo&){std::wcerr << L"block alloc\n"; return new Statement();}
        );
        // ParsedInfo.size == Syntax.size. so, ParsedInfo[x].pv == Rule[x], if Rule[x] <: <expression> or <statement> then ParsedInfo[x].expr or ParsedInfo[x].stmt != nullptr. And if you want to extract data from ParsedInfo for your AST node - feel free.
        // <if> ::= 'if' <expression> '{' <expression> '}' 'else' '{' <expression> '}'
        par.defExpr(L"if-then-else",
                defSyntax(
                        (L"if"), expr,  block,
                        (L"else"),      block
                ), 
                [](Parser::ParseInfo & info ){
                        return new Expression();         
                }
        );
        // <when> ::= 'when' <expression> '{' <statement> '}' 
        par.defStmt(L"when",
                defSyntax(
                        (L"when"), expr,block
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
                        (L"unless"), expr,block
                ), 
                [](Parser::ParseInfo & info ){
                        std::wcerr << L"unless alloc\n";
                        return new Statement();         
                }
        );

        // new syntax :)
        // <let> ::= 'let' <identifer> '=' <expression>
        auto let = par.defStmt(L"let",
                defSyntax(
                        (L"let"),id,any(id),(L"="),expr
                ),
                [](Parser::ParseInfo&){
                        std::wcerr << L"let alloc\n";
                        return new Statement();
                }
        );

        // <def> ::= 'def' <identifer> '=' <expression>
        par.defStmt(L"def",
                defSyntax(
                        (L"def"),id,any(id),(L"="),expr
                ),
                [](Parser::ParseInfo&){
                        std::wcerr << L"def alloc\n";
                        return new Statement();
                }
        );

        // <for> ::= 'for' <let> ('let' <identifer> '=' <expression>) 'to' <expression> 'do' <statement>
        par.defStmt(L"for", 
                defSyntax(
                        (L"for"),let,(L"to"),expr,(L"do"),block
                ),
                [](Parser::ParseInfo&){std::wcerr<<L"for alloc\n";return new Statement();
        });

        par.defExpr(L"lambda",
                defSyntax(
                        (L"\\"), id,any(id), (L"->"), expr
                ),
                [](Parser::ParseInfo&){return new Expression();}
        );

        par.defStmt(L"macro",
                defSyntax(
                        (L"macro"), id , (L"["), any(term) , // declare syntax. todo - change to inf(id|str) , all declared as <id> - macro argument.
                                (L"]"),(L"="), stmt
                ), 
                [] (Parser::ParseInfo&){std::wcerr<<L"macro alloc\n";return new Statement();}
        );
        par.defStmt(L"`unpack",
                defSyntax(
                        (L"`unpack"),(L"("),(L"<["),any(term),(L"]>"),any(id), (L")")
                ),
                [](Parser::ParseInfo&){return new Statement();}
        );
        // use - macro def ["def", #id! , #id!any , "=", #expr! ] = let id = `unpack( [\ `1  ->] ~> infid) <expression>  
                                                                // \ id -> expr - haskell lambda notation
                                                                // `unpack - unpacking arguments. like variadic template, but a bit more powerful
        // def f x y = x*y // => produces: let f = \x -> \y -> x*y
        par.showRules();

        par.addData(L"  for let i = 1 to 10 do {                                \
                                when a {                                        \
                                        unless a {                              \
                                                let c = 10.10                   \
                                        }                                       \
                                }                                               \
                                def f x y = 10                                  \
                                macro `def [ \"def\" f `i \"=\" expr ] =        \
                                        let f = \\ i -> expr                    \
                        }"

                , L"test");
        
        par.Parse();
        
        par.showError();
        return 0;
}
