#include "lexer/lexer.hpp"
#include <iostream>
#include "utils/debug.hpp"
#include "parser/parser.hpp"
using namespace lambda;
int main(int argc, char const *argv[])
{
        Parser par;

        using i = ParseVal;

        // <if> ::= 'if' <expression> 'then' <statement> 'else' <expression> 
        // syntetic <if>. not real. I'll change it later.
        par.defExpr({
                        i::Token(L"if"),  i::Expr(), 
                        i::Token(L"then"),i::Stmt(),
                        i::Token(L"else"),i::Expr()
                }, 
                [](std::vector<ParsedVal>&){
                        std::wcerr<<L"alloc if then else\n";
                        return new Expression();
                }
        );

        // <when> ::= 'when' <expression> <statement> 
        // if without else.
        par.defStmt({
                        i::Token(L"when"),i::Expr(),i::Stmt()      
                }, 
                [](std::vector<ParsedVal>&){
                        std::wcerr<<L"alloc when\n";
                        return new Statement();
                }
        );

        // <let> ::= 'let' <identifer> '=' <expression>
        par.defStmt({   
                        i::Token(L"let"),i::Ident(), i::Token(L"="), i::Expr()
                }, 
                [](std::vector<ParsedVal>&){
                        std::wcerr << L"alloc let\n"; 
                        return new Statement();
                }
        );

        // <block> ::=  '{' <statement>* '}' 
        // like C/C++ blocks;
        par.defStmt(i::Token(L"{"),[](Parser & par, Lexer& lex){
                auto id =lex.TryRecognize(L"}");
                while(lex.nextTok().id!=id){
                        std::wcerr << L"tok:" << lex.lastTok().val << L' '<< lex.lastTok().id << L"\n";
                        par.expectStatement(false);
                }
                return new Statement();
        });

        // haskell lambda notation. only one argument.(for more then one - use carrying)
        // <lambda> ::= '\' <identifer> '->' <expression>
        // \ x -> <expression>
        par.defExpr({
                        i::Token(L"\\"),i::Ident(),i::Token(L"->"),i::Expr()
                },  
                [](std::vector<ParsedVal>&){
                        std::wcerr << L"alloc h.lambda\n"; 
                        return new Expression();
                }
        );
        //and also mathematic lambda notation.
        // <lambda> ::= 'λ' <identifer> '.' <expression>
        par.defExpr({
                        i::Token(L"λ"),i::Ident(),i::Token(L"."),i::Expr()
                },  
                [](std::vector<ParsedVal>&){
                        std::wcerr << L"alloc m.lambda\n"; 
                        return new Expression();
                }
        );

        par.showRules();

        // simple test.
        par.addData(L"\\ x -> \
                        \\ y -> \
                                if x then\
                                        {\
                                                when y \
                                                        let x = y\
                                                let y = x\
                                        }\
                                else λx.λy.10",
                L"test");

        par.Parse();

        par.showError(); //if error is empty then all is ok.(or possible all is wrong but error output is empty)

        return 0;
}