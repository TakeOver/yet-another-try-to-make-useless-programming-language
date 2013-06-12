#include "lexer/lexer.hpp"
#include <iostream>
#include "utils/debug.hpp"
#include "parser/parser.hpp"
using namespace lambda;
int main(int argc, char const *argv[])
{
        Parser par;

        // <if> ::= 'if' <expr> 'then' <stmt> 'else' <expr> // syntetic <if>. not real. I'll change it later.
        par.defExpr({
                ParseVal::Token(L"if"), ParseVal::Expr(), 
                ParseVal::Token(L"then"), ParseVal::Stmt(),
                ParseVal::Token(L"else"), ParseVal::Expr()
        }, [](std::vector<ParsedVal>&){std::wcerr<<L"alloc if then else\n";return new Expression();});

        // <when> ::= 'when' <expr> <stmt> // if without else.
        par.defStmt({
                ParseVal::Token(L"when"),ParseVal::Expr(),
                        ParseVal::Stmt()      
        }, [](std::vector<ParsedVal>&){std::wcerr<<L"alloc when\n";return new Statement();});

        // <let> ::= 'let' <identifer> '=' <expr>
        par.defStmt({   
                ParseVal::Token(L"let"),ParseVal::Ident(), ParseVal::Token(L"="), ParseVal::Expr()
        }, [](std::vector<ParsedVal>&){std::wcerr << L"alloc let\n"; return new Statement();});

        // <block> ::=  '{' <stmt>* '}' // like C/C++ blocks;
        par.defStmt(ParseVal::Token(L"{"),[](Parser & par, Lexer& lex){
                auto id =lex.TryRecognize(L"}");
                while(lex.nextTok().id!=id){
                        std::wcerr << L"tok:" << lex.lastTok().val << L' '<< lex.lastTok().id << L"\n";
                        par.expectStatement(false);
                }
                return new Statement();
        });
        using i = ParseVal;
        par.defExpr({
                i::Token(L"\\"),
                i::Ident(),
                i::Token(L"->"),
                i::Expr()
        },  [](std::vector<ParsedVal>&){std::wcerr << L"alloc lambda\n"; return new Expression();});
        par.showRules();
        par.addData(L"\\ x -> \
                        \\ y -> \
                                if x then\
                                        {\
                                                when y \
                                                        let x = y\
                                                let y = x\
                                        }\
                                else x",
                L"test");
        par.Parse();
        par.showError(); //if error is empty then all is ok.(or possible all is wrong but error output is empty)
        return 0;
}