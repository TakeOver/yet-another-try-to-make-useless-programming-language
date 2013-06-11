#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <cassert>
#include "token.hpp"
#include "stream.hpp"
#include <functional>
namespace lambda{
        class Lexer{

                std::unordered_map<std::wstring, uint32_t> keywords;

                mutable uint16_t last_tok_id = static_cast<uint16_t>(Token::__END_OF_TOKENS);
                mutable uint16_t last_user_tok_id = last_tok_id;
                
                std::unordered_map<uint32_t, std::wstring> user_tokens_si;
                std::unordered_map<std::wstring,uint32_t> user_tokens_is;

                mutable uint16_t max_kwd_length = 0;

                std::wstring filename;

                FileStream fs;

                std::function<void(FileStream&)> commentsDelegate;

                bool failed = false;
                std::wstring error;

                static constexpr auto is_dig = FileStream::is_dig;
                static constexpr auto is_letter = FileStream::is_letter;
                static constexpr auto is_whitespace = FileStream::is_whitespace;
                
                void HandleError(std::wstring msg, tok_info_t ti){
                        std::wstring ws = L"Syntax error: "+msg + L" at line:" + std::to_wstring(ti.line) + L" in file:" + fs.curfile() + L"\n";
                        ws += fs.datastring(ti.pos) + L"\n";
                        int space = 10;
                        while(--space)
                                ws+=L"~";
                        failed = true;
                        error = ws;
                }
                
                token_t getNum(){
                        std::wstring val;
                        tok_info_t ti ( fs.line(),fs.position());
                        auto const nt = Token::NUM;
                        wchar_t wc;
                        while(!fs.eos() && (is_dig(wc=fs.curChar(false)) || wc == L'.')){
                                fs.nextChar(false);
                                val+=wc;
                        }
                        return token_t(nt,ti,val);
                }
                
                token_t getChar(){
                        std::wstring val;
                        tok_info_t ti ( fs.line(),fs.position());
                        auto const nt = Token::CHAR;
                        wchar_t wc = fs.nextChar();
                        if(!fs.eos() && fs.nextChar()!='\'')
                                HandleError(L"Expected end of char.", ti);
                        fs.nextChar();
                        val = wc;
                        return token_t(nt,ti,val);
                }
                
                token_t getString(){
                        std::wstring val;
                        tok_info_t ti ( fs.line(),fs.position());
                        auto const nt = Token::STRING;
                        wchar_t wc = 0;
                        while(!fs.eos() && wc != L'\"'){
                                wc = fs.nextChar();
                                val+=wc;
                        }
                        if(wc!=L'\"')
                                HandleError(L"End eof string expected", ti);
                        fs.nextChar();
                        return token_t(nt,ti,val);
                }
                
                token_t getIdent(){
                        std::wstring val;
                        tok_info_t ti ( fs.line(),fs.position());
                        auto const nt = Token::IDENTIFER;
                        wchar_t wc;
                        while(!fs.eos() && (is_letter(wc=fs.curChar()) || is_dig(wc))){
                                fs.nextChar();
                                val+=wc;
                        }
                        return token_t(nt,ti,val);
                }

                token_t getOperator(){
                        std::wstring val, matched;
                        tok_info_t ti ( fs.line(),fs.position());
                        auto const nt = Token::OPERATOR;
                        wchar_t wc;
                        uint32_t lim = max_kwd_length;
                        while(!fs.eos() && (fs.recognizeToken() == nt) && lim){
                                wc = fs.curChar();
                                fs.nextChar(false);
                                val+=wc;
                                auto iter = keywords.find(val);
                                if(iter!=keywords.end())
                                        matched = val;
                                --lim;
                        }
                        fs.retChars(max_kwd_length - matched.size() - lim);
                        return token_t(nt,ti,matched);
                }

                void TrimWhiteSpaceAndComments(){
                        while(!fs.eof() && is_whitespace(fs.curChar())){
                                fs.nextChar();
                        }
                        if(commentsDelegate == nullptr)
                                return;
                        commentsDelegate(fs);
                        
                        TrimWhiteSpaceAndComments();
                }
                token_t _nextTok(){
                        TrimWhiteSpaceAndComments();
                        switch(fs.recognizeToken()){
                                case Token::NUM:        return getNum();
                                case Token::STRING:     return getString();
                                case Token::CHAR:       return getChar();
                                case Token::IDENTIFER:  return getIdent();
                                case Token::OPERATOR:   return getOperator();
                                default:                return token_t(Token::NONE,{0,0},L"EOF");
                        }
                }

        public: 
                std::wstring getError(){
                        return error;
                }
                Lexer(std::wstring data,std::wstring filename){
                        fs.push(data, filename);
                }

                token_t nextTok(){
                        auto tok = _nextTok();
                        auto iter = keywords.find(tok.val);
                        if(iter == keywords.end()){
                                tok.id = static_cast<uint16_t>(tok.tok);
                        }else{
                                tok.id = iter->second;
                        }
                        return tok;
                }
                uint32_t defToken(std::wstring name){
                        user_tokens_si[last_user_tok_id] = name;
                        user_tokens_is[name] = last_user_tok_id;
                        keywords[name] = last_user_tok_id;
                        max_kwd_length = std::max((uint32_t)max_kwd_length,(uint32_t)name.length());
                        return last_user_tok_id++;
                }


        };
}