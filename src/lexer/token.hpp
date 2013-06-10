#pragma once
#include <string>
#include <cstdint>

namespace lambda{
        namespace token{
                enum tok_t{
                        EOF_T = 0, 
                        ERROR_T, 
                        NONE_T,
                        NUM_T,
                        STR_T,
                        OP_T,
                        IDENT_T
                };
        }
        typedef token::tok_t tok_t; 
        // this struct is used for error information, because of that fact that AST nodes do not know about parser, so errors 
        // in codegen process should handle more information.
        struct tok_info_t {
                uint32_t line = 0; 
                uint32_t pos = 0;
                std::wstring filename; 
                tok_info_t (uint32_t l, uint32_t p, std::wstring s): line(l), pos(p), filename(s){}
        };
        
        struct token_t{
                tok_t tok;
                tok_info_t tokinfo;
                std::wstring val;
                std::wstring str;  // for more useful errors handling.
                token_t(tok_t t, tok_info_t ti, std::wstring s, std::wstring v): tok(t), tokinfo(ti),
                val(v), str(s){}
        };
}