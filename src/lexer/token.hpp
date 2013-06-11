#pragma once
#include <string>
#include <cstdint>

namespace lambda{
        enum class Token{
                NONE = 0,
                END, 
                ERROR,
                NUM,
                STRING,
                CHAR,
                OPERATOR,
                IDENTIFER,
                __END_OF_TOKENS
        };
        // this struct is used for error information, because of that fact that AST nodes do not know about parser, so errors 
        // in codegen process should handle more information.
        struct tok_info_t {
                uint32_t line = 0; 
                uint32_t pos = 0;
                tok_info_t (uint32_t l, uint32_t p): line(l), pos(p){}
        };
        
        struct token_t{
                Token tok;
                tok_info_t tokinfo;
                std::wstring val;
                token_t(Token t, tok_info_t ti, std::wstring v): tok(t), tokinfo(ti),
                val(v){}
        };
}