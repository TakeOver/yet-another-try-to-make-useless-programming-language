
#define DEBUG
#include "lexer/lexer.hpp"
#include <iostream>
#include "utils/json.hpp"
#include "utils/debug.hpp"
#include "parser/parser.hpp"
using namespace lambda;
int main(int argc, char const *argv[])
{
        Lexer * lex = new Lexer(L"123.3 qwerty_`wr \n+\n-\n\n*\n#//\n/'1234",L"test");
        lex->defToken(L"+");
        lex->defToken(L"-");
        lex->defToken(L"/");
        lex->defToken(L"//");
        lex->defToken(L"*");
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        DBG_TRACE("%s %d", "test",10);
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
                        ParseVal::Expr()      
        }, [](std::vector<ParsedVal>&){std::wcerr<<L"alloc when\n";return new Statement();});
        par.showRules();
        par.addData(L"if 0 then when 0 1 else 1",L"test");
        par.Parse();
        par.showError(); //if error is empty then all is ok.
        return 0;
}