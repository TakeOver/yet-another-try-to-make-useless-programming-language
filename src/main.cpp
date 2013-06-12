
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
        par.defRule({
                        ParseVal::defToken(L"if"), 
                                ParseVal::Expr(), 
                        ParseVal::defToken(L"then"), 
                                ParseVal::Expr(),
                        ParseVal::defToken(L"else"),
                                ParseVal::Expr()});
        par.showRules();
        par.addData(L"if 0 then  1 else 1",L"test");
        par.Parse();
        par.showError(); //if error is empty then all is ok.
        return 0;
}