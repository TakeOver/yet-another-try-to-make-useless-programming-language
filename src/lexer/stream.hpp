#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <cassert>
#include <unordered_map>
#include "token.hpp"
namespace lambda{
        class FileStream{
        private:
                std::vector<std::wstring> data;
                std::vector<std::wstring> filenames;
                std::vector<uint32_t> positions;
                std::vector<uint32_t> lines;

                std::vector< uint32_t > states;
                
                std::unordered_map<uint32_t, std::wstring> user_tokens;

        public:
                void pop(){                        
                        lines.pop_back();
                        positions.pop_back();
                        data.pop_back();
                        filenames.pop_back();
                }
                std::wstring datastring(uint32_t line){
                        return data.at(line);
                }
                std::wstring curfile(){
                        return filenames.back();
                }
                std::wstring curline(){
                        return data.at(positions.back());
                }
                uint32_t line(){
                        return lines.back();
                }
                uint32_t position(){
                        return positions.back();
                }
                bool eof(){
                        return positions.size() == 1;
                }
                bool eos(){
                        return !eof() && data.size()> positions.back();
                }
                wchar_t curChar(bool allow_change = true){
                        if(eof())
                                return '\0';
                        if(eos() && allow_change){
                                pop();
                                return curChar();
                        }else if(eos())
                                return '\0';
                        return data.back()[positions.back()];
                }
                static bool is_dig(wchar_t wc){
                        return (wc>=L'0' && wc<=L'9');
                } 
                static bool is_letter(wchar_t wc){
                        return (wc>=L'a' && wc<=L'z')
                        ||(wc>=L'A' && wc<=L'Z')
                        ||(wc>=L'а' && wc<=L'я')
                        ||(wc>=L'А' && wc<=L'я') 
                        || wc == L'_' || wc == L'`';
                } 
                static bool is_whitespace(wchar_t wc){
                        return wc == L' ' || wc == L'\t' || wc == L'\r' || wc == L'\n';
                }
                void retChars(uint32_t num){
                        positions.back()-=num;
                }
                Token recognizeToken(bool allow_change = true){
                        wchar_t wc = curChar(allow_change);
                        if(is_dig(wc))
                                return Token::NUM;
                        if(is_letter(wc))
                                return Token::IDENTIFER;
                        if(wc == L'\"')
                                return Token::STRING;
                        if(wc == L'\'')
                                return Token::CHAR;
                        if(wc == L'\0')
                                return Token::NONE;
                        assert(!is_whitespace(wc) && "control white space. stream.hpp");
                        return Token::OPERATOR;

                }
                wchar_t nextChar(bool allow_change = true){
                        auto tmp = curChar(allow_change);
                        if(tmp)
                                positions.back()++;
                        if(tmp == L'\n')
                                lines.back()++;
                        return tmp;
                }
                void push(std::wstring data, std::wstring filename){
                        this->data.push_back(data), 
                        filenames.push_back(filename), 
                        positions.push_back(0), 
                        lines.push_back(1);
                }
        };
}