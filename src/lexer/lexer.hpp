#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <cassert>
#include "token.hpp"
#include "charstack.hpp"
namespace lambda{
        class Lexer{
                std::unordered_map<std::wstring, token_t> kwds;
                mutable uint16_t maxkwd = 0;
                CharStack cs;
                std::vector<token_t> temp_tokens; // for lookahead
                uint32_t cur_tok = 0;
                token_t getNumber(){
                        std::wstring str;
                        while(!cs.eof() && !cs.eol() && ! cs.eot(CharStack::NUM_T)){
                                str += cs.getChar();
                        }
                        return token_t(token::NUM_T,tok_info_t(cs.Line(),cs.Pos(),cs.filenames.back()),cs.curStr(),str);
                }
                token_t getOp(){
                        std::wstring s, matched;
                        uint lim = maxkwd;
                        while(--lim && !cs.eol() && !cs.eof() && ! cs.eot(CharStack::OP_T)){
                                s+=cs.getChar();
                                auto iter = kwds.find(s);
                                if(iter!=kwds.end()){
                                        matched = s;
                                }
                        }
                        if(matched.length() < maxkwd)
                                cs.retSymbols(maxkwd - matched.length());
                        return token_t(token::OP_T,tok_info_t(cs.Line(),cs.Pos(),cs.filenames.back()),cs.curStr(),s);                        
                }
                token_t getString(){
                        wchar_t term = cs.getChar();
                        wchar_t ch;
                        std::wstring str;
                        while(!cs.eof() && !cs.eol() && term!=(ch=cs.getChar())){
                                if(ch == L'\\'){
                                        switch(cs.getChar()){
                                                case L'\\':str+=L'\\';break;
                                                case L'n':str+=L'\n';break;
                                                case L't':str+=L'\t';break;
                                                case L'\'':str+=L'\'';break;
                                                case L'\"':str+=L'\"';break;
                                                case L'a':str+=L'\a';break;
                                                case L'f':str+=L'\f';break;
                                                case L'r':str+=L'\r';break;
                                                case L'b':str+=L'\b';break;
                                                default: assert(false&&"Failed to escape");
                                        }
                                }
                                str+=ch;
                        }
                        return token_t(token::STR_T,tok_info_t(cs.Line(),cs.Pos(),cs.filenames.back()),cs.curStr(),str); 
                }
                token_t getIdent(){
                        std::wstring str;
                        while(!cs.eof() && !cs.eol() && ! (cs.eot(CharStack::LETTER_T) || cs.eot(CharStack::NUM_T))){
                                str += cs.getChar();
                        }
                        return token_t(token::IDENT_T,tok_info_t(cs.Line(),cs.Pos(),cs.filenames.back()),cs.curStr(),str);
                }
        public:
                token_t nextTok(){
                        if(cur_tok<temp_tokens.size())
                                return temp_tokens[cur_tok++];
                        auto match = cs.recognize_type();
                        token_t tok (token::NONE_T,{0,0,L""},L"",L"");
                        if(match == CharStack::NUM_T)
                                tok = getNumber();
                        else if(match == CharStack::LETTER_T)
                                tok = getIdent();
                        else if(match == CharStack::OP_T)
                                tok = getOp();
                        else tok = getString();
                        temp_tokens.push_back(tok);
                        cur_tok++;
                        return tok;
                }
                token_t lookNext(){
                        auto tok = nextTok();
                        cur_tok--;
                        return tok;
                }
                void retTok(){
                        cur_tok--;
                }

        };
}