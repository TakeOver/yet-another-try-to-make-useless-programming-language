#include "lexer/lexer.hpp"
#include <iostream>
#include "utils/json.hpp"
#include "utils/debug.hpp"
#include "parser/parser.hpp"
using namespace lambda;
int main(int argc, char const *argv[])
{
        Parser par;
        par.defExpr({
                ParseVal::Token(L"if"), 
                        ParseVal::Expr(), 
                ParseVal::Token(L"then"), 
                        ParseVal::Stmt(),
                ParseVal::Token(L"else"),
                        ParseVal::Expr()
        }, [](std::vector<ParsedVal>&){std::wcerr<<L"alloc if then else\n";return new Expression();});

        par.defStmt({
                ParseVal::Token(L"when"),
                        ParseVal::Expr(),
                        ParseVal::Stmt()      
        }, [](std::vector<ParsedVal>&){std::wcerr<<L"alloc when\n";return new Statement();});

        par.defStmt({   
                ParseVal::Token(L"let"),ParseVal::Ident(), ParseVal::Token(L"="), ParseVal::Expr()
        }, [](std::vector<ParsedVal>&){std::wcerr << L"alloc let\n"; return new Statement();});

        par.showRules();
        par.addData(L"if 0 then \nwhen 0 \nlet a = +\n else 1",
                L"test");
        par.Parse();
        par.showError(); //if error is empty then all is ok.
        return 0;
}