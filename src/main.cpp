
#include "lexer/lexer.hpp"
#include <iostream>
#include "utils/json.hpp"
using namespace lambda;
int main(int argc, char const *argv[])
{
        Lexer * lex = new Lexer(L"123.3 qwerty_`wr \n+\n-\n\n*\n#//\n/'1234",L"test");
        lex->defToken(L"+");
        lex->defToken(L"-");
        lex->defToken(L"/");
        lex->defToken(L"//");
        lex->defToken(L"*");
        std::wcout<< L"HelloWorld\n";
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->nextTok().val << std::endl;
        std::wcout << lex->getError() << std::endl;
        JsonReader jr ("json.json");
        std::wcout << jr.get(L"value1", false) << L'\n';
        std::wcout << jr.get(L"value2", std::wstring(L"fail!")) << L'\n';
        std::wcout << jr.get(L"value3", 42.0) << L'\n';
        auto value4 = jr.getChild(L"value4");
        std::wcout << value4.get(L"value5", L"") << L'\n'; // so, boost::ptree ranslates null to string. :C
        std::wcout << value4.get(L"value6",13) << L'\n';
        std::wcout << value4.get(L"value7", L"fail!11221") << L'\n';
        JsonWriter jw;
        jw.add(L"SUPERPROPERTY",false);
        jw.add(L"TEST",42);
        jw.add(L"LOL",std::wstring(L"LOLI"));
        jw.addChild(L"123", jr.getPtree());
        jw.write2File("jsonnew.json");
        JsonWriter jw1(jr.getPtree());
        jw1.write2File("newjson.json");
        return 0;
}