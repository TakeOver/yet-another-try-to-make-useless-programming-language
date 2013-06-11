
#include "lexer/lexer.hpp"
#include <iostream>
using namespace lambda;
int main(int argc, char const *argv[])
{
        Lexer * lex = new Lexer(L"123.3 qwerty_`wr +-/*",L"test");
        lex->defToken(L"+");
        lex->defToken(L"-");
        lex->defToken(L"/");
        lex->defToken(L"*");
        std::wcout<< L"HelloWorld\n";
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        return 0;
}