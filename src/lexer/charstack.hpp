#pragma once
#include <string>
#include <vector>
namespace lambda{
        struct CharStack{
                std::vector<std::wstring> data;
                std::vector<std::wstring> filenames;
                std::vector<uint32_t> positions; 
                uint32_t line = 0;
                enum char_type_t {
                        ERROR_T, NUM_T, LETTER_T, OP_T ,STR_T
                };
                static bool is_digit(wchar_t wc){
                        return wc>= L'0' && wc<= L'9';
                }
                void trim(){
                        wchar_t ch;
                        while (eol() && ((ch = this->getChar()) == L' ' || ch == L'\n' || ch == L'\t')){
                                if(ch == L'\n')
                                        line++;
                                positions.back()++;
                        }
                        if(eol()){
                                positions.pop_back();
                                filenames.pop_back();
                                data.pop_back();
                                line = 0;
                                trim();
                        }
                }
                void retSymbols(uint32_t i){
                        positions.back()-=i;
                }
                static bool is_letter(wchar_t wc){
                        return (wc>=L'a' && wc <= L'z')
                                || (wc>=L'A' && wc <= L'Z')
                                || (wc>=L'а' && wc <= L'я')
                                || (wc>=L'А' && wc <= L'Я')
                                ||  (wc == L'_');
                }
                bool eol(){
                        return positions.back() >= data.back().length();
                }
                bool eof(){
                        return positions.empty();
                }
                bool eot( char_type_t t){
                        return t!= recognize_type(false);
                }
                uint32_t Line(){return line;}
                uint32_t Pos(){return positions.back();}
                wchar_t getChar(){
                        if (eof())
                                return 0;
                        if (eol())
                                return '\n';
                        wchar_t ch = data.back()[positions.back()++];
                        return ch;
                }
                std::wstring& curStr(){return data.back();}

                char_type_t recognize_type(bool begin = true){
                        wchar_t ch = this->getChar();                        
                        if(is_digit(ch) && (!begin && ch == L'.'))
                                return NUM_T;
                        if(is_letter(ch))
                                return LETTER_T;
                        if(ch == L'\'' || ch == L'\"')
                                return STR_T;
                        return OP_T;
                }
        };
}