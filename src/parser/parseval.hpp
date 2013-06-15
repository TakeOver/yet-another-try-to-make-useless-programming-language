#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../lexer/token.hpp"
#include "../ast/statement.hpp"
#include "../ast/expression.hpp"

namespace lambda{
        enum class ParseValType{
                Token = 0,
                Expression,
                Statement,
                Identifer,
                Rule,
                String,
                Operator,
                Number,
                Term
        };
        
        struct ParseVal{
                
                ParseValType type;
                uint16_t token_id = 0;
                uint32_t rule_id = 0;
                bool any = false; // i want to reduce 'inf*' parse-types. and make code less ugly
                std::wstring val; // i'll put String/Number/Operator in val.

                ParseVal(ParseValType t, std::wstring val): type(t), val(val){}
                ParseVal(ParseValType t):type(t){}
                ParseVal(ParseValType t, uint32_t id):type(t), rule_id(id){}
                inline static ParseVal Token(std::wstring val){
                        return ParseVal(ParseValType::Token, val);
                }
                inline static ParseVal Expr() {
                        return ParseVal(ParseValType::Expression);
                }
                inline static ParseVal Stmt(){
                        return ParseVal(ParseValType::Statement);
                }
                inline static ParseVal Any(ParseVal what){
                        what.any = true;
                        return what;
                }
                inline static ParseVal Ident(){
                        return ParseVal(ParseValType::Identifer);
                }
                inline static ParseVal Rule(uint32_t id){
                        return ParseVal(ParseValType::Rule,id);
                }
                inline static ParseVal Str(){
                        return ParseVal(ParseValType::String);
                }
                inline static ParseVal Oper(){
                        return ParseVal(ParseValType::Operator);
                }
                inline static ParseVal Num(){
                        return ParseVal(ParseValType::Number);
                }
                inline static ParseVal Term(){
                        return ParseVal(ParseValType::Term);
                }

        };
        struct ParsedVal{
                ParseVal pv;     

                union{
                        Statement  ** stmt = nullptr;
                        Expression ** expr = nullptr;
                };
                uint32_t size = 0;

                std::vector<token_t> vals;

                ParsedVal(ParseVal pv, Statement**stmt, uint32_t size = 1): pv(pv), stmt(stmt), size(size){}
                ParsedVal(ParseVal pv, Expression**expr, uint32_t size = 1): pv(pv), expr(expr), size(size){}
                ParsedVal(ParseVal pv): pv(pv),expr(nullptr){}
                ParsedVal(ParseVal pv,std::vector<token_t> && vals):pv(pv), vals(vals){}
                ParsedVal(ParseVal pv,std::vector<token_t> vals):pv(pv), vals(vals){}
        };
}
